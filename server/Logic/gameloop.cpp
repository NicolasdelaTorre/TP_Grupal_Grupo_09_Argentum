#include "gameloop.h"

#include <cstdlib>
#include <iostream>
#include <memory>

#include "../../common/Communication/events/client_events.h"
#include "../../common/Communication/events/server_events.h"
#include "../../common/Communication/message_types.h"

#include "NPC/creature.h"

// Mapea el nombre del NPC (del toml) al byte de NpcCode que espera el cliente
// (enum NpcCode en common/DTOs.h). Si no matchea, devuelve 0 (SPIDER) como fallback.
static uint8_t npcTypeFromName(const std::string& name) {
    if (name == "spider") return 0;
    if (name == "skeleton") return 1;
    if (name == "zombie") return 2;
    if (name == "goblin") return 3;
    if (name == "orc") return 4;
    if (name == "golem") return 5;
    return 0;
}

// Broadcast un ItemDroppedEvent por cada drop que generó una muerte.
static void broadcastDrops(ClientMonitor& monitor,
                           const std::vector<Game::DroppedItemRecord>& drops) {
    for (const auto& d: drops) {
        monitor.broadcast(std::make_shared<ItemDroppedEvent>(d.dropId, d.itemId, d.x, d.y,
                                                             d.goldAmount));
    }
}

// dx/dy → byte de dirección en formato wire (3=TOP, 4=BOTTOM, 5=LEFT, 6=RIGHT).
static uint8_t wireDirFromDelta(int16_t dx, int16_t dy) {
    if (std::abs(dx) >= std::abs(dy)) {
        if (dx > 0) return static_cast<uint8_t>(MoveDirection::RIGHT);
        if (dx < 0) return static_cast<uint8_t>(MoveDirection::LEFT);
    }
    if (dy > 0) return static_cast<uint8_t>(MoveDirection::BOTTOM);
    if (dy < 0) return static_cast<uint8_t>(MoveDirection::TOP);
    return static_cast<uint8_t>(MoveDirection::BOTTOM);
}

Gameloop::Gameloop(IncomingQueue& clientEvents, ClientMonitor& clientMonitor, Map& map,
                   ServerProtocol& protocol):
        clientEvents(clientEvents),
        clientMonitor(clientMonitor),
        gameFinished(false),
        map(map),
        game(map),
        protocol(protocol),
        turnManager(game.getPlayerIds(), map.getAllNPCIds(), map, game) {}

void Gameloop::sendMapSnapshot(int playerId, uint8_t mapId) {
    std::vector<MapCellData> cells;
    cells.reserve(map.getCellCount(mapId));
    for (size_t i = 0; i < map.getCellCount(mapId); i++) {
        Cell c = map.getCell(i, mapId);
        cells.push_back({c.textureId, c.obstacleId, c.safeZone});
    }

    std::vector<MapObstacleData> obstacles;
    const auto& placed = map.getObstacles(mapId);
    obstacles.reserve(placed.size());
    for (const auto& o: placed) {
        obstacles.push_back({o.type, o.x, o.y, o.w, o.h});
    }

    clientMonitor.sendToClient(playerId,
                               std::make_shared<MapEvent>(map.getWidth(mapId), map.getHeight(mapId),
                                                          std::move(cells), std::move(obstacles)));
}

void Gameloop::sendNpcSnapshot(int playerId, uint8_t mapId) {
    // Criaturas (hostiles) que viven en el mapId pedido. Le mandamos también los
    // muertos (alive=false) para que el id quede registrado de cara a respawns.
    for (uint16_t npcId: map.getAllNPCIds()) {
        Creature* npc = map.getNPC(npcId);
        if (!npc || npc->getMapId() != mapId)
            continue;
        Position np = npc->getPosition();
        uint8_t type = npcTypeFromName(npc->getName());
        bool alive = !npc->isDead();
        clientMonitor.sendToClient(
                playerId, std::make_shared<NewNpcEvent>(npcId, np.x, np.y, type, alive));
    }

    // NPCs amigos (merchant/banker/priest): solo viven en las ciudades del overworld.
    if (mapId == 0) {
        for (const auto& f: map.getFriendlyNpcs()) {
            clientMonitor.sendToClient(
                    playerId, std::make_shared<NewNpcEvent>(f.id, f.x, f.y, f.type, /*alive=*/true));
        }
    }
}

void Gameloop::run() {
    while (!gameFinished) {
        std::shared_ptr<ClientEvent> ev;
        while (clientEvents.try_pop(ev)) {
            dispatch(*ev);
        }

        turnManager.updateTimers();
        PlayerTurns();
        NPCTurns();

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

// dispatch del ClientEvent al handler correspondiente.
void Gameloop::dispatch(const ClientEvent& ev) {
    int pid = ev.getPlayerId();
    if (auto* p = dynamic_cast<const UserArrivalEvent*>(&ev)) {
        handleUserArrival(pid, p->getName(), p->getRace(), p->getClass());
    } else if (auto* p = dynamic_cast<const MovementEvent*>(&ev)) {
        handleMovement(pid, p->getDirection());
    } else if (auto* p = dynamic_cast<const TurnEvent*>(&ev)) {
        handleTurn(pid, p->getDirection());
    } else if (auto* p = dynamic_cast<const SkinSelectedEvent*>(&ev)) {
        handleSkinSelected(pid, p->getSkinId());
    } else if (auto* p = dynamic_cast<const HeadSelectedEvent*>(&ev)) {
        handleHeadSelected(pid, p->getHeadId());
    } else if (auto* p = dynamic_cast<const AttackEvent*>(&ev)) {
        handleAttack(pid, p->getTargetType(), p->getTargetId());
    } else if (dynamic_cast<const PickUpItemEvent*>(&ev)) {
        handlePickUp(pid);
    } else if (auto* p = dynamic_cast<const DropItemEvent*>(&ev)) {
        handleDrop(pid, p->getInvSlot());
    } else if (auto* p = dynamic_cast<const EquipItemEvent*>(&ev)) {
        handleEquip(pid, p->getInvSlot());
    } else if (auto* p = dynamic_cast<const UnequipItemEvent*>(&ev)) {
        handleUnequip(pid, p->getSlotType());
    } else if (auto* p = dynamic_cast<const ChatMessageEvent*>(&ev)) {
        handleChat(pid, p->getText());
    } else if (auto* p = dynamic_cast<const SelectNpcEvent*>(&ev)) {
        handleSelectNpc(pid, p->getNpcId());
    } else if (dynamic_cast<const DisconnectEvent*>(&ev)) {
        handleDisconnect(pid);
    }
}

// Tick por player (corre cada frame del loop). Hoy solo restaura mana a
// jugadores meditando. Cuando agreguemos más estados con timers (regeneración
// de vida, debuffs, etc.) viven acá.
void Gameloop::PlayerTurns() {
    std::vector<int> playerIds = game.getPlayerIds();
    turnManager.addPlayers(playerIds);
    turnManager.removePlayers(playerIds);

    // Verify if a player is teleporting to a city
    for (int playerId : playerIds) {
        if (game.hasPlayer(playerId) && game.checkIfPlayerIsTeleporting(playerId) && !turnManager.alreadyTeleporting(playerId)) {
            int timeToTeleport = map.calculateTeleportingTime(game.getPlayerPosition(playerId), game.getPlayerMapId(playerId));

            if (timeToTeleport == -1) continue;

            turnManager.setTimeToTeleport(playerId, timeToTeleport);
        }
    }

    // Get Players ready to restore health
    std::vector<int> playersToRestoreHealth = turnManager.getPlayersReadyToRestoreHealth();
    for (int playerId: playersToRestoreHealth) {
        game.restorePlayerHealth(playerId);
        clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
    }

    // Get Players ready to restore mana through time
    std::vector<int> playersToRestoreManaThroughTime = turnManager.getPlayersReadyToRestoreManaThroughTime();
    for (int playerId: playersToRestoreManaThroughTime) {
        game.restorePlayerMana(playerId);
        clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
    }

    // Get Players ready to restore mana by meditating
    std::vector<int> playersToRestoreMana = turnManager.getPlayersReadyToRestoreManaByMeditation();
    for (int playerId: playersToRestoreMana) {
        game.restorePlayerManaForMeditation(playerId);
        // Cada vez que recuperamos mana mandamos stats actualizados.
        clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
    }

    // Get Players ready to teleport to a city
    std::vector<int> playersToTeleport = turnManager.getPlayersReadyToTeleport();
    for (int playerId : playersToTeleport) {
        game.finishTeleportingState(playerId);
        Position playerPosition = {0, 0};
        uint8_t mapId = game.getPlayerMapId(playerId);
        if (mapId > 0) {
            playerPosition = map.getEntryPosition(mapId);
        } else {
            playerPosition = game.getPlayerPosition(playerId);
        }

        if (playerPosition.x == -1 || playerPosition.y == -1) {
            continue;
        }

        Position priestPosition = map.searchNearestPriest(playerPosition.x, playerPosition.y);

        map.moveEntity(playerId, playerPosition.x, playerPosition.y, priestPosition.x, priestPosition.y + 1, true, 0);
        game.fastTravel(playerId, {priestPosition.x, (int16_t)(priestPosition.y + 1)});

        auto r = game.revivePlayer(playerId);
        std::string reply = r.message;
        if (r.ok) {
            Position p = game.getPlayerPosition(playerId);
            clientMonitor.broadcast(std::make_shared<PlayerRevivedEvent>(
                    static_cast<uint16_t>(playerId), p.x, p.y));
        }

        clientMonitor.sendToClient(playerId,
                               std::make_shared<ChatBroadcastEvent>(0, std::string(), reply));
        clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
    }
}

void Gameloop::NPCTurns() {
    // NPCs que toca mover: persiguen al jugador más cercano. Si se movieron,
    // broadcast NpcMovedEvent con dirección calculada desde el delta.
    std::vector<uint16_t> npcsToMove = turnManager.getNPCsReady(true);
    for (uint16_t npcId: npcsToMove) {
        Creature* npc = map.getNPC(npcId);

        if (npc->isDead())
            continue;

        Position oldPos = npc->getPosition();
        Position newPos = npc->stalkPlayer(map.searchPlayer(oldPos.x, oldPos.y, npc->getMapId()));
        
        if (newPos.x == -1 || (newPos.x == oldPos.x && newPos.y == oldPos.y))
            continue;
        if (map.moveEntity(npcId, oldPos.x, oldPos.y, newPos.x, newPos.y, false, npc->getMapId())) {
            npc->move(newPos);

            // Solo el overworld viaja al cliente (mapId=0).
            if (npc->getMapId() != 0)
                continue;
            uint8_t dir = wireDirFromDelta(static_cast<int16_t>(newPos.x - oldPos.x),
                                        static_cast<int16_t>(newPos.y - oldPos.y));
            clientMonitor.broadcast(std::make_shared<NpcMovedEvent>(npcId, newPos.x, newPos.y, dir));
        }
    }

    // NPCs que toca atacar: si tienen un jugador adyacente, le aplican daño y
    // notifican al cliente afectado con sus stats actualizados.
    std::vector<uint16_t> npcsToAttack = turnManager.getNPCsReady(false);
    for (uint16_t npcId: npcsToAttack) {
        Creature* npc = map.getNPC(npcId);

        if (npc->isDead())
            continue;

        uint8_t playerId = map.nextEntity(npc->getPosition().x, npc->getPosition().y, true,
                                          npc->getMapId());
        if (playerId == 0)
            continue;
        if (game.applyNPCAttack(playerId, npc->getDamage())) {
            auto atkEv = std::make_shared<AttackResultEvent>(npcId, /*targetType=*/0, playerId,
                                                            npc->getDamage(), /*hit=*/true);
            clientMonitor.broadcast(atkEv);
            clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
            // Si el NPC mató al jugador, broadcast PlayerDiedEvent + drop loot.
            if (game.isPlayerGhost(playerId)) {
                clientMonitor.broadcast(std::make_shared<PlayerDiedEvent>(playerId));
                broadcastDrops(clientMonitor, game.dropPlayerLootOnDeath(playerId));
                auto snap = game.getInventorySnapshot(playerId);
                clientMonitor.sendToClient(playerId,
                                           std::make_shared<InventoryUpdateEvent>(
                                                   snap.items, snap.equippedWeapon,
                                                   snap.equippedArmor, snap.equippedHelmet,
                                                   snap.equippedShield));
                clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
            }
        }
    }

    // NPCs que revivan tras el cooldown: broadcast NpcRespawnedEvent para que
    // el cliente vuelva a mostrarlos.
    std::vector<uint16_t> npcsToRevive = turnManager.reviveNPCs();
    for (uint16_t npcId: npcsToRevive) {
        Creature* npc = map.getNPC(npcId);

        npc->resurrect();

        Position randomPosition = map.getRandomPosition(npc->getBiomeType(), npc->getMapId());
        map.placeEntity(npcId, randomPosition.x, randomPosition.y, npc->getMapId(), false);
        npc->move(randomPosition);

        clientMonitor.broadcast(std::make_shared<NpcRespawnedEvent>(npcId, randomPosition.x, randomPosition.y));
    }
}

void Gameloop::stop() { gameFinished = true; }

// ── Handlers ─────────────────────────────────────────────────────────────

void Gameloop::handleDisconnect(int playerId) {
    if (!game.hasPlayer(playerId))
        return;
    std::cout << "Player " << game.getPlayerName(playerId) << " (id=" << playerId
              << ") disconnected" << std::endl;
    selectedNpc.erase(playerId);
    game.removePlayer(playerId);
    clientMonitor.broadcastExcept(
            playerId, std::make_shared<PlayerDisconnectedEvent>(static_cast<uint16_t>(playerId)));
}

void Gameloop::handleUserArrival(int playerId, const std::string& name, RaceCode race,
                                 ClassCode class_) {
    bool success = game.addPlayer(playerId, name, race, class_);
    uint8_t opcode = success ? static_cast<uint8_t>(ServerMsg::FIRST_LOGIN)
                             : static_cast<uint8_t>(ServerMsg::LOGIN_FAIL);
    clientMonitor.sendToClient(playerId, std::make_shared<OpcodeOnlyEvent>(opcode));
}

void Gameloop::handleSkinSelected(int playerId, uint8_t skinId) {
    if (!game.hasPlayer(playerId))
        return;

    Position p = game.getPlayerPosition(playerId);
    game.setSkin(playerId, skinId);
    clientMonitor.sendToClient(playerId, std::make_shared<LoginOkEvent>(p.x, p.y));

    sendMapSnapshot(playerId, 0);

    // Stats iniciales.
    clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));

    // Inventario inicial: sin esto el cliente arranca con el panel vacío y no
    // ve los items persistidos hasta que cambie algo (pickup/drop/equip).
    {
        auto snap = game.getInventorySnapshot(playerId);
        clientMonitor.sendToClient(playerId,
                                   std::make_shared<InventoryUpdateEvent>(
                                           snap.items, snap.equippedWeapon, snap.equippedArmor,
                                           snap.equippedHelmet, snap.equippedShield));
    }

    // Mandarle un NEW_PLAYER por cada jugador que ya estaba + sus PLAYER_EQUIPPED.
    // Si alguno está como fantasma, también su PlayerDiedEvent para que el
    // cliente lo dibuje como fantasma desde el arranque.
    for (int otherId: game.getPlayerIds()) {
        if (otherId == playerId)
            continue;
        Position op = game.getPlayerPosition(otherId);
        const std::string& oname = game.getPlayerName(otherId);
        uint8_t odir = game.getPlayerDirection(otherId);
        uint8_t oskin = game.getPlayerSkin(otherId);
        clientMonitor.sendToClient(
                playerId, std::make_shared<NewPlayerEvent>(static_cast<uint16_t>(otherId), op.x,
                                                          op.y, odir, oskin, oname));
        sendEquipmentSnapshot(otherId, playerId);
        if (game.isPlayerGhost(otherId)) {
            clientMonitor.sendToClient(
                    playerId, std::make_shared<PlayerDiedEvent>(static_cast<uint16_t>(otherId)));
        }
    }

    // Avisarles a los demás del recién llegado + su vestimenta.
    const std::string& myName = game.getPlayerName(playerId);
    uint8_t myDir = game.getPlayerDirection(playerId);
    uint8_t mySkin = game.getPlayerSkin(playerId);
    clientMonitor.broadcastExcept(playerId,
                                  std::make_shared<NewPlayerEvent>(static_cast<uint16_t>(playerId),
                                                                   p.x, p.y, myDir, mySkin, myName));
    sendEquipmentSnapshot(playerId, -1);
    if (game.isPlayerGhost(playerId)) {
        // Estado persistido en .bin: vuelve fantasma al loguearse.
        clientMonitor.broadcastExcept(
                playerId, std::make_shared<PlayerDiedEvent>(static_cast<uint16_t>(playerId)));
        clientMonitor.sendToClient(
                playerId, std::make_shared<PlayerDiedEvent>(static_cast<uint16_t>(playerId)));
    }

    // Snapshot de NPCs del overworld (mapId=0): el cliente los renderiza. Incluye
    // hostiles (vivos y muertos) y amigos (merchant/banker/priest).
    sendNpcSnapshot(playerId, 0);

    // Snapshot de items en el piso. Mandamos un ItemDroppedEvent por cada uno;
    // así el cliente unifica el code path con los drops que llegan en vivo.
    for (const auto& d : game.getDroppedItems()) {
        clientMonitor.sendToClient(
                playerId, std::make_shared<ItemDroppedEvent>(d.dropId, d.itemId, d.x, d.y));
    }
}

void Gameloop::handleHeadSelected(int /*playerId*/, uint8_t /*headId*/) {
    // TODO(team-gameplay): guardar headSkinId y reenviarlo en NEW_PLAYER cuando
    // se implemente la renderización de cabeza separada del cuerpo.
}

void Gameloop::handleMovement(int playerId, MoveDirection direction) {
    const uint8_t previousMapId = game.getPlayerMapId(playerId);
    if (game.movePlayer(playerId, direction)) {
        Position p = game.getPlayerPosition(playerId);
        uint8_t pdir = game.getPlayerDirection(playerId);
        const uint8_t currentMapId = game.getPlayerMapId(playerId);
        if (currentMapId != previousMapId) {
            // El cliente, al recibir el MapEvent, limpia jugadores/NPCs/items, así
            // que después le reenviamos los NPCs del nuevo mapa.
            sendMapSnapshot(playerId, currentMapId);
            sendNpcSnapshot(playerId, currentMapId);
            clientMonitor.sendToClient(playerId,
                                       std::make_shared<MoveRejectedEvent>(p.x, p.y));
            return;
        }
        clientMonitor.broadcastExcept(playerId,
                                      std::make_shared<PlayerMovedEvent>(
                                              static_cast<uint16_t>(playerId), p.x, p.y, pdir));
    } else {
        // El server rechazo la prediccion del cliente. Le mandamos la posicion autoritativa para que reconcilie.
        Position p = game.getPlayerPosition(playerId);
        clientMonitor.sendToClient(playerId,
                                   std::make_shared<MoveRejectedEvent>(p.x, p.y));
    }
}

void Gameloop::handleTurn(int playerId, MoveDirection direction) {
    bool success = game.turnPlayer(playerId, direction);
    if (success) {
        Position p = game.getPlayerPosition(playerId);
        uint8_t pdir = game.getPlayerDirection(playerId);
        clientMonitor.broadcastExcept(playerId,
                                      std::make_shared<PlayerMovedEvent>(
                                              static_cast<uint16_t>(playerId), p.x, p.y, pdir));
    }
}

void Gameloop::handleAttack(int playerId, uint8_t targetType, uint16_t targetId) {
    if (!game.hasPlayer(playerId))
        return;

    // Un fantasma no puede atacar a nadie.
    if (game.isPlayerGhost(playerId)) {
        clientMonitor.sendToClient(
                playerId, std::make_shared<ChatBroadcastEvent>(
                                  0, std::string(), "Estás muerto, no podés atacar"));
        return;
    }

    auto ev = game.processAttack(playerId, targetType, targetId);
    if (!ev) {
        std::cout << "ATTACK from player=" << playerId << " ttype=" << (int)targetType
                  << " tid=" << targetId << " not performed" << std::endl;
        return;
    }
    std::cout << "ATTACK from player=" << playerId << " ttype=" << (int)targetType
              << " tid=" << targetId << " hit=" << ev->getHit() << " dmg=" << ev->getDamage()
              << std::endl;
    clientMonitor.broadcast(ev);
    // El atacante gano exp con processAttack
    if (ev->getHit()) {
        clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
    }
    if (ev->getHit() && ev->getTargetType() == 0 && game.hasPlayer(ev->getTargetId())) {
        clientMonitor.sendToClient(ev->getTargetId(), buildStatsEvent(ev->getTargetId()));
        // Si el ataque mató al target, broadcast PlayerDiedEvent + drop loot
        // (inventario + oro en exceso).
        if (game.isPlayerGhost(ev->getTargetId())) {
            clientMonitor.broadcast(std::make_shared<PlayerDiedEvent>(ev->getTargetId()));
            broadcastDrops(clientMonitor, game.dropPlayerLootOnDeath(ev->getTargetId()));
            // El inventario y el oro cambiaron: re-mandar snapshot + stats.
            auto snap = game.getInventorySnapshot(ev->getTargetId());
            clientMonitor.sendToClient(ev->getTargetId(),
                                       std::make_shared<InventoryUpdateEvent>(
                                               snap.items, snap.equippedWeapon, snap.equippedArmor,
                                               snap.equippedHelmet, snap.equippedShield));
            clientMonitor.sendToClient(ev->getTargetId(), buildStatsEvent(ev->getTargetId()));
        }
    }
    // Si pegamos a un NPC y lo matamos, broadcast NpcDiedEvent + drop loot
    // (probabilidades del enunciado).
    if (ev->getHit() && ev->getTargetType() == 1) {
        Creature* npc = map.getNPC(ev->getTargetId());
        if (npc && npc->isDead()) {
            clientMonitor.broadcast(std::make_shared<NpcDiedEvent>(ev->getTargetId()));
            broadcastDrops(clientMonitor, game.dropCreatureLootOnDeath(ev->getTargetId()));
        }
    }
}

// ── Comandos de inventario ───────────────────────────────────────────────
//
// Cada uno (pickup/drop/equip/unequip) puede cambiar el inventario y/o lo
// equipado. Después del cambio: INVENTORY_UPDATE al dueño y, si cambió un
// slot equipado, PLAYER_EQUIPPED a los demás.

static void broadcastInventoryChanges(int playerId, const Game::InventorySnapshot& before,
                                      const Game::InventorySnapshot& after, Game& /*game*/,
                                      ClientMonitor& clientMonitor) {
    // INVENTORY_UPDATE al dueño.
    clientMonitor.sendToClient(playerId,
                               std::make_shared<InventoryUpdateEvent>(
                                       after.items, after.equippedWeapon, after.equippedArmor,
                                       after.equippedHelmet, after.equippedShield));

    // PLAYER_EQUIPPED broadcast por cada slot equipado que cambió.
    const uint8_t beforeSlots[4] = {before.equippedWeapon, before.equippedArmor,
                                    before.equippedHelmet, before.equippedShield};
    const uint8_t afterSlots[4] = {after.equippedWeapon, after.equippedArmor,
                                   after.equippedHelmet, after.equippedShield};
    for (uint8_t s = 0; s < 4; s++) {
        if (beforeSlots[s] != afterSlots[s]) {
            clientMonitor.broadcastExcept(
                    playerId, std::make_shared<PlayerEquippedEvent>(
                                      static_cast<uint16_t>(playerId), s, afterSlots[s]));
        }
    }
}

void Gameloop::handlePickUp(int playerId) {
    if (!game.hasPlayer(playerId))
        return;
    if (game.isPlayerGhost(playerId)) {
        clientMonitor.sendToClient(
                playerId, std::make_shared<ChatBroadcastEvent>(
                                  0, std::string(), "Estás muerto, no podés levantar items"));
        return;
    }
    auto before = game.getInventorySnapshot(playerId);
    auto r = game.pickUpItemAt(playerId);
    clientMonitor.sendToClient(
            playerId, std::make_shared<ChatBroadcastEvent>(0, std::string(), r.message));
    if (!r.ok) return;
    // Broadcast a todos que ese drop ya no está en el piso + actualizamos
    // inventario del dueño.
    clientMonitor.broadcast(std::make_shared<ItemPickedUpEvent>(r.record.dropId));
    auto after = game.getInventorySnapshot(playerId);
    broadcastInventoryChanges(playerId, before, after, game, clientMonitor);
    // Si era oro, el inventory snapshot no cambia → mandamos stats para que
    // el HUD del dueño refresque la cantidad de oro.
    if (r.record.itemId == Game::GOLD_ITEM_ID) {
        clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
    }
}

void Gameloop::handleDrop(int playerId, uint8_t invSlot) {
    if (!game.hasPlayer(playerId))
        return;
    if (game.isPlayerGhost(playerId)) {
        clientMonitor.sendToClient(
                playerId, std::make_shared<ChatBroadcastEvent>(
                                  0, std::string(), "Estás muerto, no podés tirar items"));
        return;
    }
    auto before = game.getInventorySnapshot(playerId);
    auto r = game.dropItem(playerId, invSlot);
    clientMonitor.sendToClient(
            playerId, std::make_shared<ChatBroadcastEvent>(0, std::string(), r.message));
    if (!r.ok) return;
    // Broadcast a todos que apareció un item nuevo en el piso.
    clientMonitor.broadcast(std::make_shared<ItemDroppedEvent>(
            r.record.dropId, r.record.itemId, r.record.x, r.record.y));
    auto after = game.getInventorySnapshot(playerId);
    broadcastInventoryChanges(playerId, before, after, game, clientMonitor);
}

void Gameloop::handleEquip(int playerId, uint8_t invSlot) {
    if (!game.hasPlayer(playerId))
        return;

    if (game.isPlayerGhost(playerId)) {
        clientMonitor.sendToClient(
                playerId, std::make_shared<ChatBroadcastEvent>(
                                  0, std::string(), "Estás muerto, no podés equiparte"));
        return;
    }
    auto before = game.getInventorySnapshot(playerId);
    if (!game.equipOrUseItem(playerId, invSlot)) {
        clientMonitor.sendToClient(
                playerId, std::make_shared<ChatBroadcastEvent>(0, std::string(),
                                                              "No se pudo equipar ese slot"));
        return;
    }
    clientMonitor.sendToClient(
            playerId, std::make_shared<ChatBroadcastEvent>(0, std::string(), "Item equipado"));
    auto after = game.getInventorySnapshot(playerId);
    broadcastInventoryChanges(playerId, before, after, game, clientMonitor);
}

void Gameloop::handleUnequip(int playerId, uint8_t slotType) {
    if (!game.hasPlayer(playerId))
        return;
    if (game.isPlayerGhost(playerId)) {
        clientMonitor.sendToClient(
                playerId, std::make_shared<ChatBroadcastEvent>(
                                  0, std::string(), "Estás muerto, no podés desequiparte"));
        return;
    }
    auto before = game.getInventorySnapshot(playerId);
    if (!game.unequipSlot(playerId, slotType)) {
        clientMonitor.sendToClient(
                playerId, std::make_shared<ChatBroadcastEvent>(
                                  0, std::string(), "No tenías ese slot equipado"));
        return;
    }
    clientMonitor.sendToClient(
            playerId, std::make_shared<ChatBroadcastEvent>(0, std::string(), "Item desequipado"));
    auto after = game.getInventorySnapshot(playerId);
    broadcastInventoryChanges(playerId, before, after, game, clientMonitor);
}

// ── Helpers privados ─────────────────────────────────────────────────────

std::shared_ptr<ServerEvent> Gameloop::buildStatsEvent(int idPlayer) {
    return std::make_shared<StatsEvent>(
            game.getPlayerHealth(idPlayer), game.getPlayerMaxHealth(idPlayer),
            game.getPlayerMana(idPlayer), game.getPlayerMaxMana(idPlayer),
            game.getPlayerGold(idPlayer), game.getPlayerExperience(idPlayer),
            game.getPlayerNextLevelExp(idPlayer), game.getPlayerLevel(idPlayer));
}

void Gameloop::sendEquipmentSnapshot(int idPlayer, int recipientId) {
    auto snap = game.getInventorySnapshot(idPlayer);
    const uint8_t slots[4] = {snap.equippedWeapon, snap.equippedArmor, snap.equippedHelmet,
                              snap.equippedShield};
    for (uint8_t s = 0; s < 4; s++) {
        if (slots[s] == 0)
            continue;
        auto ev = std::make_shared<PlayerEquippedEvent>(static_cast<uint16_t>(idPlayer), s,
                                                        slots[s]);
        if (recipientId < 0) {
            clientMonitor.broadcastExcept(idPlayer, ev);
        } else {
            clientMonitor.sendToClient(recipientId, ev);
        }
    }
}

// ── Selección de NPC amigo ───────────────────────────────────────────────

// Distancia máxima en la que un jugador puede interactuar con un NPC amigo. 2 = cualquier tile dentro de un cuadrado de 5x5 centrado en él.
static constexpr int MAX_INTERACTION_DISTANCE = 2;

static const char* friendlyKindName(uint8_t type) {
    if (type == static_cast<uint8_t>(NpcCode::MERCHANT)) return "Comerciante";
    if (type == static_cast<uint8_t>(NpcCode::BANKER)) return "Banquero";
    if (type == static_cast<uint8_t>(NpcCode::PRIEST)) return "Sacerdote";
    return "NPC";
}

void Gameloop::handleSelectNpc(int playerId, uint16_t npcId) {
    if (!game.hasPlayer(playerId))
        return;
    const FriendlyNpc* f = map.getFriendlyNpc(npcId);
    if (!f) {
        // No es un amigo. Click sobre hostil → ignoramos (el click sobre
        // hostiles ya se manda como AttackEvent desde el cliente).
        return;
    }
    Position p = game.getPlayerPosition(playerId);
    int dist = map.friendlyNpcDistance(p.x, p.y, npcId);
    if (dist < 0 || dist > MAX_INTERACTION_DISTANCE) {
        clientMonitor.sendToClient(
                playerId,
                std::make_shared<ChatBroadcastEvent>(0, std::string(),
                                                    "Estás demasiado lejos de " + f->name));
        return;
    }
    selectedNpc[playerId] = npcId;
    std::string msg = "Hablás con " + f->name + " (" + friendlyKindName(f->type) + ")";
    clientMonitor.sendToClient(
            playerId, std::make_shared<ChatBroadcastEvent>(0, std::string(), std::move(msg)));
}

// ── Chat ─────────────────────────────────────────────────────────────────

void Gameloop::handleChat(int playerId, const std::string& text) {
    if (text.empty())
        return;
    if (text[0] == '/') {
        handleChatCommand(playerId, text);
        return;
    }
    if (text[0] == '@') {
        handlePrivateMessage(playerId, text);
        return;
    }
    const std::string& name = game.getPlayerName(playerId);
    std::cout << "CHAT " << name << "(" << playerId << "): " << text << std::endl;
    clientMonitor.broadcast(
            std::make_shared<ChatBroadcastEvent>(static_cast<uint16_t>(playerId), name, text));
}

// Mensaje privado: "@nick mensaje". Se busca al destinatario por nombre, se le
// manda el texto con prefijo [priv] y al emisor una copia tambien con prefijo
// para confirmar que se envio.
void Gameloop::handlePrivateMessage(int playerId, const std::string& text) {
    // text[0] == '@'. Buscar el primer espacio para cortar nick / mensaje.
    size_t space = text.find(' ');
    if (space == std::string::npos || space <= 1) {
        clientMonitor.sendToClient(playerId, std::make_shared<ChatBroadcastEvent>(
                0, std::string(), "Uso: @<nick> <mensaje>"));
        return;
    }
    std::string targetName = text.substr(1, space - 1);
    // Saltar espacios consecutivos antes del mensaje.
    size_t msgStart = text.find_first_not_of(' ', space);
    if (msgStart == std::string::npos) {
        clientMonitor.sendToClient(playerId, std::make_shared<ChatBroadcastEvent>(
                0, std::string(), "Uso: @<nick> <mensaje>"));
        return;
    }
    std::string msg = text.substr(msgStart);

    const std::string& senderName = game.getPlayerName(playerId);
    if (targetName == senderName) {
        clientMonitor.sendToClient(playerId, std::make_shared<ChatBroadcastEvent>(
                0, std::string(), "No te podes mandar mensajes a vos mismo"));
        return;
    }

    // Buscar destinatario por nombre entre los conectados.
    int targetId = -1;
    for (int pid : game.getPlayerIds()) {
        if (game.getPlayerName(pid) == targetName) { targetId = pid; break; }
    }
    if (targetId == -1) {
        clientMonitor.sendToClient(playerId, std::make_shared<ChatBroadcastEvent>(
                0, std::string(), targetName + " no esta conectado"));
        return;
    }

    std::cout << "PRIV " << senderName << " -> " << targetName << ": " << msg << std::endl;
    // Al destinatario: viene del emisor (authorId = sender) con prefijo [priv].
    clientMonitor.sendToClient(targetId, std::make_shared<ChatBroadcastEvent>(
            static_cast<uint16_t>(playerId), senderName, "[priv] " + msg));
    // Al emisor: confirmacion (autoria del sistema).
    clientMonitor.sendToClient(playerId, std::make_shared<ChatBroadcastEvent>(
            0, std::string(), "[priv a " + targetName + "] " + msg));
}

// Helper: parte "cmd arg1 arg2 ..." en palabras (ignora espacios consecutivos).
static std::vector<std::string> splitWords(const std::string& s) {
    std::vector<std::string> out;
    size_t i = 0, n = s.size();
    while (i < n) {
        while (i < n && s[i] == ' ') i++;
        size_t j = i;
        while (j < n && s[j] != ' ') j++;
        if (j > i) out.push_back(s.substr(i, j - i));
        i = j;
    }
    return out;
}

void Gameloop::handleChatCommand(int playerId, const std::string& text) {
    auto parts = splitWords(text);
    if (parts.empty()) return;
    const std::string& cmd = parts[0];
    std::cout << "CHAT_CMD player=" << playerId << " cmd='" << cmd << "' args=" << (parts.size() - 1)
              << std::endl;

    bool refreshStats = false;
    bool refreshInventory = false;
    std::string reply;

    if (cmd == "/vidainf") {
        reply = game.cheatToggleInfiniteHealth(playerId) ? "Vida infinita toggled" : "Error";
    } else if (cmd == "/manainf") {
        reply = game.cheatToggleInfiniteMana(playerId) ? "Mana infinito toggled" : "Error";
    } else if (cmd == "/suicidio") {
        if (game.cheatSuicide(playerId)) {
            reply = "Te suicidaste";
            refreshStats = true;
            refreshInventory = true;
            if (game.isPlayerGhost(playerId)) {
                clientMonitor.broadcast(std::make_shared<PlayerDiedEvent>(playerId));
                broadcastDrops(clientMonitor, game.dropPlayerLootOnDeath(playerId));
            }
        } else reply = "Error";
    } else if (cmd == "/levelup") {
        if (game.cheatLevelUp(playerId)) { reply = "Subiste de nivel"; refreshStats = true; }
        else reply = "Error";
    } else if (cmd == "/gold") {
        if (parts.size() < 2) { reply = "Uso: /gold <cantidad>"; }
        else {
            try {
                uint32_t n = static_cast<uint32_t>(std::stoul(parts[1]));
                if (game.cheatAddGold(playerId, n)) {
                    reply = "Recibiste " + std::to_string(n) + " de oro";
                    refreshStats = true;
                } else reply = "Error";
            } catch (...) { reply = "Uso: /gold <cantidad>"; }
        }
    } else if (cmd == "/item") {
        if (parts.size() < 2) { reply = "Uso: /item <itemId>"; }
        else {
            try {
                uint8_t id = static_cast<uint8_t>(std::stoul(parts[1]));
                if (game.cheatSpawnItem(playerId, id)) {
                    reply = "Item " + std::to_string(id) + " agregado";
                    refreshInventory = true;
                } else reply = "Error";
            } catch (...) { reply = "Uso: /item <itemId>"; }
        }
    } else if (cmd == "/meditar") {
        auto r = game.meditatePlayer(playerId);
        reply = r.message;
    } else if (cmd == "/tomar") {
        // /tomar levanta lo que haya en la celda del jugador. El reply lo
        // arma handlePickUp (incluye ChatBroadcastEvent + ItemPickedUpEvent).
        handlePickUp(playerId);
        return;
    } else if (cmd == "/tirar") {
        // /tirar <slot>. TODO(team-ui-nico): cuando haya UI de selección de
        // inventario, llamar sin args y usar selectedSlot local del cliente.
        if (parts.size() < 2) {
            reply = "Uso: /tirar <slot>";
        } else {
            try {
                uint8_t slot = static_cast<uint8_t>(std::stoul(parts[1]));
                handleDrop(playerId, slot);
                return;
            } catch (...) { reply = "Uso: /tirar <slot>"; }
        }
    } else if (cmd == "/equipar") {
        if (parts.size() < 2) {
            reply = "Uso: /equipar <slot>";
        } else {
            try {
                uint8_t slot = static_cast<uint8_t>(std::stoul(parts[1]));
                handleEquip(playerId, slot);
                return;
            } catch (...) { reply = "Uso: /equipar <slot>"; }
        }
    } else if (cmd == "/desequipar") {
        if (parts.size() < 2) {
            reply = "Uso: /desequipar <arma|armor|casco|escudo>";
        } else {
            const std::string& which = parts[1];
            uint8_t slotType = 255;
            if (which == "arma") slotType = 0;
            else if (which == "armor" || which == "armadura") slotType = 1;
            else if (which == "casco") slotType = 2;
            else if (which == "escudo") slotType = 3;
            if (slotType == 255) {
                reply = "Uso: /desequipar <arma|armor|casco|escudo>";
            } else {
                handleUnequip(playerId, slotType);
                return;
            }
        }
    } else {
        // Comandos que requieren un NPC amigo seleccionado.
        auto itSel = selectedNpc.find(playerId);
        const FriendlyNpc* sel =
                itSel != selectedNpc.end() ? map.getFriendlyNpc(itSel->second) : nullptr;
        const uint8_t MERCHANT = static_cast<uint8_t>(NpcCode::MERCHANT);
        const uint8_t BANKER = static_cast<uint8_t>(NpcCode::BANKER);
        const uint8_t PRIEST = static_cast<uint8_t>(NpcCode::PRIEST);

        // Helper local: revalida la distancia al NPC seleccionado.
        auto stillNear = [&]() -> bool {
            if (!sel) return false;
            Position p = game.getPlayerPosition(playerId);
            int d = map.friendlyNpcDistance(p.x, p.y, sel->id);
            return d >= 0 && d <= MAX_INTERACTION_DISTANCE;
        };
        auto joinFrom = [&](size_t from) -> std::string {
            std::string out;
            for (size_t i = from; i < parts.size(); i++) {
                if (i > from) out += ' ';
                out += parts[i];
            }
            return out;
        };

        if (cmd == "/listar") {
            if (!sel || !stillNear()) {
                reply = "Necesitás estar cerca de un comerciante, sacerdote o banquero (click)";
            } else if (sel->type == MERCHANT || sel->type == PRIEST) {
                auto lines = game.listMerchantInventory(sel->type);
                for (const auto& l : lines) {
                    clientMonitor.sendToClient(
                            playerId, std::make_shared<ChatBroadcastEvent>(0, std::string(), l));
                }
                reply = "Fin de la lista";
            } else if (sel->type == BANKER) {
                auto lines = game.listBankAccount(playerId, sel->type);
                for (const auto& l : lines) {
                    clientMonitor.sendToClient(
                            playerId, std::make_shared<ChatBroadcastEvent>(0, std::string(), l));
                }
                reply = "Fin de la lista";
            } else {
                reply = "Este NPC no tiene nada para listar";
            }
        } else if (cmd == "/comprar") {
            if (parts.size() < 2) { reply = "Uso: /comprar <objeto>"; }
            else if (!sel || !stillNear()) { reply = "No hay comerciante seleccionado cerca"; }
            else if (sel->type != MERCHANT && sel->type != PRIEST) {
                reply = "Sólo podés comprarle a un comerciante o sacerdote";
            } else {
                auto r = game.buyFromNpc(playerId, sel->type, joinFrom(1));
                reply = r.message;
                if (r.ok) { refreshStats = true; refreshInventory = true; }
            }
        } else if (cmd == "/vender") {
            if (parts.size() < 2) { reply = "Uso: /vender <objeto>"; }
            else if (!sel || !stillNear() || sel->type != MERCHANT) {
                reply = "No hay comerciante seleccionado cerca";
            } else {
                auto r = game.sellToNpc(playerId, sel->type, joinFrom(1));
                reply = r.message;
                if (r.ok) { refreshStats = true; refreshInventory = true; }
            }
        } else if (cmd == "/depositar") {
            if (parts.size() < 2) { reply = "Uso: /depositar <objeto> | /depositar oro <cant>"; }
            else if (!sel || !stillNear() || sel->type != BANKER) {
                reply = "No hay banquero seleccionado cerca";
            } else if (parts[1] == "oro") {
                if (parts.size() < 3) { reply = "Uso: /depositar oro <cant>"; }
                else {
                    try {
                        uint32_t n = static_cast<uint32_t>(std::stoul(parts[2]));
                        auto r = game.depositGoldToBank(playerId, n);
                        reply = r.message;
                        if (r.ok) refreshStats = true;
                    } catch (...) { reply = "Uso: /depositar oro <cant>"; }
                }
            } else {
                auto r = game.depositItemToBank(playerId, joinFrom(1));
                reply = r.message;
                if (r.ok) refreshInventory = true;
            }
        } else if (cmd == "/retirar") {
            if (parts.size() < 2) { reply = "Uso: /retirar <objeto> | /retirar oro <cant>"; }
            else if (!sel || !stillNear() || sel->type != BANKER) {
                reply = "No hay banquero seleccionado cerca";
            } else if (parts[1] == "oro") {
                if (parts.size() < 3) { reply = "Uso: /retirar oro <cant>"; }
                else {
                    try {
                        uint32_t n = static_cast<uint32_t>(std::stoul(parts[2]));
                        auto r = game.withdrawGoldFromBank(playerId, n);
                        reply = r.message;
                        if (r.ok) refreshStats = true;
                    } catch (...) { reply = "Uso: /retirar oro <cant>"; }
                }
            } else {
                auto r = game.withdrawItemFromBank(playerId, joinFrom(1));
                reply = r.message;
                if (r.ok) refreshInventory = true;
            }
        } else if (cmd == "/resucitar") {
            game.startPlayerResurrect(playerId);
            return;
        } else if (cmd == "/curar") {
            if (!sel || !stillNear() || sel->type != PRIEST) {
                reply = "No hay sacerdote seleccionado cerca";
            } else {
                auto r = game.healPlayer(playerId);
                reply = r.message;
                if (r.ok) refreshStats = true;
            }
        } else {
            reply = "Comando desconocido: " + cmd;
        }
    }

    clientMonitor.sendToClient(playerId,
                               std::make_shared<ChatBroadcastEvent>(0, std::string(), reply));
    if (refreshStats) {
        clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
    }
    if (refreshInventory) {
        auto snap = game.getInventorySnapshot(playerId);
        clientMonitor.sendToClient(playerId,
                                   std::make_shared<InventoryUpdateEvent>(
                                           snap.items, snap.equippedWeapon, snap.equippedArmor,
                                           snap.equippedHelmet, snap.equippedShield));
    }
}
