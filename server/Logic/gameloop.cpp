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

// Notifica a todos los miembros conectados del clan, salvo a exceptId si != -1.
static void notifyClanMembers(ClientMonitor& monitor, Game& game, const std::string& clanName,
                              const std::string& msg, int exceptId = -1) {
    if (clanName.empty())
        return;
    auto members = game.getClanMembers(clanName);
    for (int pid: game.getPlayerIds()) {
        if (pid == exceptId)
            continue;
        const std::string& name = game.getPlayerName(pid);
        for (const auto& m: members) {
            if (m == name) {
                monitor.sendToClient(pid, std::make_shared<ChatBroadcastEvent>(
                                                  0, std::string(), msg));
                break;
            }
        }
    }
}

// Manda las notificaciones de combate
static void notifyAttackOutcome(ClientMonitor& monitor, Game& game,
                                const Game::AttackOutcome& o) {
    if (!o.valid) return;

    // Atacante: qué le hizo al target.
    std::string toAttacker;
    if (o.evaded) {
        toAttacker = o.targetName + " esquivó tu ataque";
    } else if (o.critical) {
        toAttacker = "¡Crítico! Le hiciste " + std::to_string(o.damage) +
                     " de daño a " + o.targetName;
    } else {
        toAttacker = "Le hiciste " + std::to_string(o.damage) + " de daño a " + o.targetName;
    }
    monitor.sendToClient(o.attackerId,
                         std::make_shared<ChatBroadcastEvent>(0, std::string(), toAttacker));

    if (o.killed) {
        monitor.sendToClient(o.attackerId,
                             std::make_shared<ChatBroadcastEvent>(
                                     0, std::string(), "Mataste a " + o.targetName));
    }
    if (o.leveledUp) {
        monitor.sendToClient(o.attackerId,
                             std::make_shared<ChatBroadcastEvent>(
                                     0, std::string(),
                                     "¡Subiste a nivel " + std::to_string(o.newLevel) + "!"));
    }

    // Target: solo si es player (NPCs no leen chat).
    if (o.targetType != 0 || o.targetId < 0) return;
    std::string toTarget;
    if (o.evaded) {
        toTarget = "Esquivaste el ataque de " + o.attackerName;
    } else if (o.critical) {
        toTarget = o.attackerName + " te pegó un crítico por " + std::to_string(o.damage);
    } else {
        toTarget = o.attackerName + " te atacó por " + std::to_string(o.damage);
    }
    monitor.sendToClient(o.targetId,
                         std::make_shared<ChatBroadcastEvent>(0, std::string(), toTarget));
    if (o.killed) {
        monitor.sendToClient(o.targetId,
                             std::make_shared<ChatBroadcastEvent>(
                                     0, std::string(), "Fuiste asesinado por " + o.attackerName));
    }

    // Aviso al clan del target: "tu compañero esta siendo atacado".
    std::string clanOfTarget = game.getClanOf(o.targetId);
    if (!clanOfTarget.empty()) {
        std::string msg = "Tu compañero " + o.targetName + " está siendo atacado";
        notifyClanMembers(monitor, game, clanOfTarget, msg, o.targetId);
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
        obstacles.push_back({o.x, o.y, o.w, o.h, o.texture});
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
        handleUserArrival(pid, p->getName());
    } else if (auto* p = dynamic_cast<const CharacterCreatedEvent*>(&ev)) {
        handleCharacterCreated(pid, p->getRace(), p->getClass(), p->getHeadId(), p->getSkinId());
    } else if (auto* p = dynamic_cast<const MovementEvent*>(&ev)) {
        handleMovement(pid, p->getDirection());
    } else if (auto* p = dynamic_cast<const TurnEvent*>(&ev)) {
        handleTurn(pid, p->getDirection());
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
        Position target = map.searchPlayer(oldPos.x, oldPos.y, npc->getMapId(), npc->getBiomeType());
        // Si el player es fantasma no se persigue
        if (target.x != -1) {
            int pid = game.getPlayerIdAt(target.x, target.y, npc->getMapId());
            if (pid != -1 && game.isPlayerGhost(pid)) {
                target = {-1, -1};
            }
        }
        Position newPos = npc->stalkPlayer(target);

        if (newPos.x == -1 || (newPos.x == oldPos.x && newPos.y == oldPos.y))
            continue;
        // Zona segura: los NPCs hostiles no pueden entrar.
        if (map.isSafeZone(newPos.x, newPos.y, npc->getMapId()))
            continue;
        if (map.moveEntity(npcId, oldPos.x, oldPos.y, newPos.x, newPos.y, false, npc->getMapId())) {
            npc->move(newPos);

            // Broadcast a todos: los ids de NPC son únicos por mapa, así que el
            // cliente solo reacciona si tiene cargado ese NPC (mismo mapa).
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
        if (!game.hasPlayer(playerId)) continue;  // ya están muertos
        // No pegar a fantasmas
        if (game.isPlayerGhost(playerId)) continue;
        uint16_t finalDmg = game.applyNPCAttack(playerId, npc->getDamage());
        auto atkEv = std::make_shared<AttackResultEvent>(npcId, 0, playerId, finalDmg, true);
        clientMonitor.broadcast(atkEv);
        clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
        clientMonitor.sendToClient(playerId,
                                   std::make_shared<ChatBroadcastEvent>(
                                           0, std::string(),
                                           npc->getName() + " te atacó por " +
                                                   std::to_string(finalDmg)));
        // Aviso al clan del jugador atacado.
        {
            std::string clanOfTarget = game.getClanOf(playerId);
            if (!clanOfTarget.empty()) {
                std::string msg = "Tu compañero " + game.getPlayerName(playerId) +
                                  " está siendo atacado";
                notifyClanMembers(clientMonitor, game, clanOfTarget, msg, playerId);
            }
        }
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
            clientMonitor.sendToClient(playerId,
                                       std::make_shared<ChatBroadcastEvent>(
                                               0, std::string(),
                                               "Te mató " + npc->getName()));
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
    // Aviso al clan antes de remover al jugador
    std::string clanName = game.getClanOf(playerId);
    std::string playerName = game.getPlayerName(playerId);
    selectedNpc.erase(playerId);
    pendingNewPlayers.erase(playerId);
    game.removePlayer(playerId);
    clientMonitor.broadcastExcept(
            playerId, std::make_shared<PlayerDisconnectedEvent>(static_cast<uint16_t>(playerId)));
    if (!clanName.empty()) {
        notifyClanMembers(clientMonitor, game, clanName,
                          playerName + " salió de Argentum", playerId);
    }
}

void Gameloop::handleUserArrival(int playerId, const std::string& name) {
    if (game.playerExistsInRecords(name)) {
        if (!game.loadExistingPlayer(playerId, name)) {
            clientMonitor.sendToClient(
                    playerId, std::make_shared<OpcodeOnlyEvent>(
                                      static_cast<uint8_t>(ServerMsg::LOGIN_FAIL)));
            return;
        }
        sendPostLoginSnapshots(playerId);
        return;
    }
    // Jugador nuevo: el cliente todavia tiene que mandar raza/clase/skin/head.
    pendingNewPlayers[playerId] = name;
    clientMonitor.sendToClient(
            playerId,
            std::make_shared<OpcodeOnlyEvent>(static_cast<uint8_t>(ServerMsg::FIRST_LOGIN)));
}

void Gameloop::handleCharacterCreated(int playerId, RaceCode race, ClassCode class_,
                                      uint8_t headId, uint8_t skinId) {
    auto it = pendingNewPlayers.find(playerId);
    if (it == pendingNewPlayers.end()) {
        // No estaba en pending: ignorar (el cliente no debería mandarlo).
        return;
    }
    std::string name = std::move(it->second);
    pendingNewPlayers.erase(it);

    if (!game.addNewPlayer(playerId, name, race, class_)) {
        clientMonitor.sendToClient(
                playerId, std::make_shared<OpcodeOnlyEvent>(
                                  static_cast<uint8_t>(ServerMsg::LOGIN_FAIL)));
        return;
    }
    game.setSkin(playerId, skinId, headId);
    // addNewPlayer ya persistio el snapshot inicial; aca refrescamos para que
    // la skin/head elegidas queden en el binario.
    game.updatePlayerData(playerId);
    sendPostLoginSnapshots(playerId);
}

void Gameloop::sendPostLoginSnapshots(int playerId) {
    Position p = game.getPlayerPosition(playerId);
    uint8_t skin = game.getPlayerSkin(playerId);
    uint8_t head = game.getPlayerHead(playerId);
    const uint8_t mapId = game.getPlayerMapId(playerId);
    clientMonitor.sendToClient(playerId, std::make_shared<LoginOkEvent>(p.x, p.y, skin, head));
    sendMapSnapshot(playerId, mapId);
    clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));

    {
        auto snap = game.getInventorySnapshot(playerId);
        clientMonitor.sendToClient(playerId,
                                   std::make_shared<InventoryUpdateEvent>(
                                           snap.items, snap.equippedWeapon, snap.equippedArmor,
                                           snap.equippedHelmet, snap.equippedShield));
    }

    for (int otherId: game.getPlayerIds()) {
        if (otherId == playerId)
            continue;
        Position op = game.getPlayerPosition(otherId);
        const std::string& oname = game.getPlayerName(otherId);
        uint8_t odir = game.getPlayerDirection(otherId);
        uint8_t oskin = game.getPlayerSkin(otherId);
        uint8_t ohead = game.getPlayerHead(otherId);
        clientMonitor.sendToClient(
                playerId, std::make_shared<NewPlayerEvent>(static_cast<uint16_t>(otherId), op.x,
                                                          op.y, odir, oskin, ohead, oname));
        sendEquipmentSnapshot(otherId, playerId);
        if (game.isPlayerGhost(otherId)) {
            clientMonitor.sendToClient(
                    playerId, std::make_shared<PlayerDiedEvent>(static_cast<uint16_t>(otherId)));
        }
    }

    const std::string& myName = game.getPlayerName(playerId);
    uint8_t myDir = game.getPlayerDirection(playerId);
    uint8_t mySkin = game.getPlayerSkin(playerId);
    uint8_t myHead = game.getPlayerHead(playerId);
    clientMonitor.broadcastExcept(
            playerId,
            std::make_shared<NewPlayerEvent>(static_cast<uint16_t>(playerId), p.x, p.y, myDir,
                                             mySkin, myHead, myName));
    sendEquipmentSnapshot(playerId, -1);
    if (game.isPlayerGhost(playerId)) {
        // Estado persistido en .bin: vuelve fantasma al loguearse.
        clientMonitor.broadcastExcept(
                playerId, std::make_shared<PlayerDiedEvent>(static_cast<uint16_t>(playerId)));
        clientMonitor.sendToClient(
                playerId, std::make_shared<PlayerDiedEvent>(static_cast<uint16_t>(playerId)));
    }

    // Snapshot de NPCs del mapa donde está el jugador: el cliente los renderiza.
    // En el overworld (mapId=0) incluye amigos (merchant/banker/priest).
    sendNpcSnapshot(playerId, mapId);

    // Aviso a los miembros del clan que entró a Argentum.
    {
        std::string clanName = game.getClanOf(playerId);
        if (!clanName.empty()) {
            notifyClanMembers(clientMonitor, game, clanName,
                              myName + " entró a Argentum", playerId);
        }
    }

    // Snapshot de items en el piso. Mandamos un ItemDroppedEvent por cada uno;
    // así el cliente unifica el code path con los drops que llegan en vivo.
    for (const auto& d : game.getDroppedItems()) {
        clientMonitor.sendToClient(
                playerId, std::make_shared<ItemDroppedEvent>(d.dropId, d.itemId, d.x, d.y));
    }
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

    // Si tiene arma de curacion equipada, el click se interpreta como cast de heal. El cliente manda AttackEvent de todas formas.
    if (game.hasHealWeaponEquipped(playerId)) {
        if (targetType == 1) {
            clientMonitor.sendToClient(playerId,
                                       std::make_shared<ChatBroadcastEvent>(
                                               0, std::string(),
                                               "No podés atacar con un hechizo de curación"));
            return;
        }
        Game::HealOutcome h = game.processHealCast(playerId, targetId);
        if (!h.valid) {
            if (!h.blockedReason.empty()) {
                clientMonitor.sendToClient(playerId,
                                           std::make_shared<ChatBroadcastEvent>(
                                                   0, std::string(), h.blockedReason));
            }
            return;
        }
        // Chat del caster y del target.
        clientMonitor.sendToClient(playerId,
                                   std::make_shared<ChatBroadcastEvent>(
                                           0, std::string(),
                                           "Curaste a " + h.targetName + " por " +
                                                   std::to_string(h.healAmount) + " de vida"));
        if (h.targetId != playerId && game.hasPlayer(h.targetId)) {
            clientMonitor.sendToClient(h.targetId,
                                       std::make_shared<ChatBroadcastEvent>(
                                               0, std::string(),
                                               h.casterName + " te curó por " +
                                                       std::to_string(h.healAmount) + " de vida"));
            clientMonitor.sendToClient(h.targetId, buildStatsEvent(h.targetId));
        }
        clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
        return;
    }

    Game::AttackOutcome outcome = game.processAttack(playerId, targetType, targetId);
    if (!outcome.valid) {
        if (!outcome.blockedReason.empty()) {
            clientMonitor.sendToClient(
                    playerId, std::make_shared<ChatBroadcastEvent>(0, std::string(),
                                                                   outcome.blockedReason));
        }
        return;
    }

    clientMonitor.broadcast(outcome.event);
    notifyAttackOutcome(clientMonitor, game, outcome);

    bool hit = outcome.event->getHit();
    if (hit) {
        clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
    }
    if (hit && outcome.targetType == 0 && game.hasPlayer(outcome.event->getTargetId())) {
        clientMonitor.sendToClient(outcome.event->getTargetId(),
                                   buildStatsEvent(outcome.event->getTargetId()));
        // Si el ataque mató al target, broadcast PlayerDiedEvent + drop loot
        // (inventario + oro en exceso).
        if (game.isPlayerGhost(outcome.event->getTargetId())) {
            clientMonitor.broadcast(std::make_shared<PlayerDiedEvent>(outcome.event->getTargetId()));
            broadcastDrops(clientMonitor,
                           game.dropPlayerLootOnDeath(outcome.event->getTargetId()));
            auto snap = game.getInventorySnapshot(outcome.event->getTargetId());
            clientMonitor.sendToClient(outcome.event->getTargetId(),
                                       std::make_shared<InventoryUpdateEvent>(
                                               snap.items, snap.equippedWeapon, snap.equippedArmor,
                                               snap.equippedHelmet, snap.equippedShield));
            clientMonitor.sendToClient(outcome.event->getTargetId(),
                                       buildStatsEvent(outcome.event->getTargetId()));
        }
    }
    if (hit && outcome.targetType == 1) {
        Creature* npc = map.getNPC(outcome.event->getTargetId());
        if (npc && npc->isDead()) {
            clientMonitor.broadcast(std::make_shared<NpcDiedEvent>(outcome.event->getTargetId()));
            broadcastDrops(clientMonitor,
                           game.dropCreatureLootOnDeath(outcome.event->getTargetId()));
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
    } else if (cmd == "/fundar-clan" || cmd == "/unirse" || cmd == "/revisar-clan" ||
               cmd == "/clan-aceptar" || cmd == "/clan-rechazar" || cmd == "/clan-ban" ||
               cmd == "/clan-kick" || cmd == "/dejar-clan") {
        // Helper local para juntar partes (nombre de clan o de nick puede tener espacios).
        auto joinFrom = [&](size_t from) -> std::string {
            std::string out;
            for (size_t i = from; i < parts.size(); i++) {
                if (i > from) out += ' ';
                out += parts[i];
            }
            return out;
        };
        const std::string& callerName = game.getPlayerName(playerId);
        if (cmd == "/fundar-clan") {
            if (parts.size() < 2) { reply = "Uso: /fundar-clan <nombre>"; }
            else {
                auto r = game.tryFoundClan(playerId, joinFrom(1));
                reply = r.message;
            }
        } else if (cmd == "/unirse") {
            if (parts.size() < 2) { reply = "Uso: /unirse <nombre del clan>"; }
            else {
                std::string clanName = joinFrom(1);
                auto r = game.tryRequestJoinClan(playerId, clanName);
                reply = r.message;
                // Si el pedido fue aceptado entrar a la cola, avisarle al fundador conectado.
                if (r.ok) {
                    auto members = game.getClanMembers(clanName);
                    if (!members.empty()) {
                        const std::string& founder = members[0];
                        for (int pid : game.getPlayerIds()) {
                            if (game.getPlayerName(pid) == founder) {
                                clientMonitor.sendToClient(
                                        pid, std::make_shared<ChatBroadcastEvent>(
                                                     0, std::string(),
                                                     callerName + " pidió unirse a tu clan"));
                                break;
                            }
                        }
                    }
                }
            }
        } else if (cmd == "/revisar-clan") {
            auto lines = game.reviewClan(playerId);
            for (const auto& l : lines) {
                clientMonitor.sendToClient(
                        playerId, std::make_shared<ChatBroadcastEvent>(0, std::string(), l));
            }
            return;
        } else if (cmd == "/clan-aceptar") {
            if (parts.size() < 2) { reply = "Uso: /clan-aceptar <nick>"; }
            else {
                std::string applicant = joinFrom(1);
                std::string clanName = game.getClanOf(playerId);
                auto r = game.tryAcceptClanRequest(playerId, applicant);
                reply = r.message;
                if (r.ok && !clanName.empty()) {
                    // Aviso al aceptado (si está conectado).
                    for (int pid : game.getPlayerIds()) {
                        if (game.getPlayerName(pid) == applicant) {
                            clientMonitor.sendToClient(
                                    pid, std::make_shared<ChatBroadcastEvent>(
                                                 0, std::string(),
                                                 "Te aceptaron en el clan " + clanName));
                            break;
                        }
                    }
                    // Aviso al resto del clan.
                    notifyClanMembers(clientMonitor, game, clanName,
                                      applicant + " entró al clan", playerId);
                }
            }
        } else if (cmd == "/clan-rechazar") {
            if (parts.size() < 2) { reply = "Uso: /clan-rechazar <nick>"; }
            else {
                std::string applicant = joinFrom(1);
                auto r = game.tryRejectClanRequest(playerId, applicant);
                reply = r.message;
                if (r.ok) {
                    for (int pid : game.getPlayerIds()) {
                        if (game.getPlayerName(pid) == applicant) {
                            clientMonitor.sendToClient(
                                    pid, std::make_shared<ChatBroadcastEvent>(
                                                 0, std::string(),
                                                 "Tu pedido al clan fue rechazado"));
                            break;
                        }
                    }
                }
            }
        } else if (cmd == "/clan-ban") {
            if (parts.size() < 2) { reply = "Uso: /clan-ban <nick>"; }
            else {
                std::string target = joinFrom(1);
                std::string clanName = game.getClanOf(playerId);
                auto r = game.tryBanFromClan(playerId, target);
                reply = r.message;
                if (r.ok) {
                    for (int pid : game.getPlayerIds()) {
                        if (game.getPlayerName(pid) == target) {
                            clientMonitor.sendToClient(
                                    pid, std::make_shared<ChatBroadcastEvent>(
                                                 0, std::string(),
                                                 "Fuiste baneado del clan " + clanName));
                            break;
                        }
                    }
                }
            }
        } else if (cmd == "/clan-kick") {
            if (parts.size() < 2) { reply = "Uso: /clan-kick <nick>"; }
            else {
                std::string target = joinFrom(1);
                std::string clanName = game.getClanOf(playerId);
                auto r = game.tryKickFromClan(playerId, target);
                reply = r.message;
                if (r.ok) {
                    // Aviso al echado y al resto del clan.
                    for (int pid : game.getPlayerIds()) {
                        if (game.getPlayerName(pid) == target) {
                            clientMonitor.sendToClient(
                                    pid, std::make_shared<ChatBroadcastEvent>(
                                                 0, std::string(),
                                                 "Te echaron del clan " + clanName));
                            break;
                        }
                    }
                    notifyClanMembers(clientMonitor, game, clanName,
                                      target + " fue echado del clan", playerId);
                }
            }
        } else if (cmd == "/dejar-clan") {
            std::string clanName = game.getClanOf(playerId);
            auto r = game.tryLeaveClan(playerId);
            reply = r.message;
            if (r.ok && !clanName.empty()) {
                notifyClanMembers(clientMonitor, game, clanName,
                                  callerName + " dejó el clan", playerId);
            }
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
