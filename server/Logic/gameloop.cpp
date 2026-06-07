#include "gameloop.h"

#include <iostream>
#include <memory>

#include "../../common/Communication/events/client_events.h"
#include "../../common/Communication/events/server_events.h"
#include "../../common/Communication/message_types.h"

#include "NPC/creature.h"

Gameloop::Gameloop(IncomingQueue& clientEvents, ClientMonitor& clientMonitor, Map& map,
                   ServerProtocol& protocol):
        clientEvents(clientEvents),
        clientMonitor(clientMonitor),
        gameFinished(false),
        map(map),
        game(map),
        protocol(protocol),
        turnManager(map.getAllNPCIds(), map) {}

void Gameloop::run() {
    while (!gameFinished) {
        std::shared_ptr<ClientEvent> ev;
        while (clientEvents.try_pop(ev)) {
            dispatch(*ev);
        }
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
    } else if (auto* p = dynamic_cast<const CheatEvent*>(&ev)) {
        handleCheat(pid, p->getCode());
    } else if (dynamic_cast<const PickUpItemEvent*>(&ev)) {
        handlePickUp(pid);
    } else if (auto* p = dynamic_cast<const DropItemEvent*>(&ev)) {
        handleDrop(pid, p->getInvSlot());
    } else if (auto* p = dynamic_cast<const EquipItemEvent*>(&ev)) {
        handleEquip(pid, p->getInvSlot());
    } else if (auto* p = dynamic_cast<const UnequipItemEvent*>(&ev)) {
        handleUnequip(pid, p->getSlotType());
    } else if (dynamic_cast<const DisconnectEvent*>(&ev)) {
        handleDisconnect(pid);
    }
}

void Gameloop::NPCTurns() {
    turnManager.updateTimers();

    // NPCs que toca mover: persiguen al jugador más cercano.
    std::vector<uint16_t> npcsToMove = turnManager.getNPCsReady(true);
    for (uint16_t npcId: npcsToMove) {
        Creature* npc = map.getNPC(npcId);
        Position newPos = npc->stalkPlayer(
                map.searchPlayer(npc->getPosition().x, npc->getPosition().y, npc->getMapId()));
        if (newPos.x != -1) {
            map.moveEntity(npcId, npc->getPosition().x, npc->getPosition().y, newPos.x, newPos.y,
                           false, npc->getMapId());
        }
    }

    // NPCs que toca atacar: si tienen un jugador adyacente, le aplican daño y
    // notifican al cliente afectado con sus stats actualizados.
    std::vector<uint16_t> npcsToAttack = turnManager.getNPCsReady(false);
    for (uint16_t npcId: npcsToAttack) {
        Creature* npc = map.getNPC(npcId);
        uint8_t playerId = map.nextEntity(npc->getPosition().x, npc->getPosition().y, true,
                                          npc->getMapId());
        if (playerId == 0)
            continue;
        if (game.applyNPCAttack(playerId, npc->getDamage())) {
            // Broadcast del resultado del ataque (attacker=npcId, target=player).
            auto atkEv = std::make_shared<AttackResultEvent>(npcId, /*targetType=*/0, playerId,
                                                            npc->getDamage(), /*hit=*/true);
            clientMonitor.broadcast(atkEv);
            // Stats actualizados al jugador afectado.
            clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
        }
    }

    // NPCs que se revivan tras el cooldown.
    std::vector<uint16_t> npcsToRevive = turnManager.reviveNPCs();
    for (uint16_t npcId: npcsToRevive) {
        Creature* npc = map.getNPC(npcId);
        npc->resurrect();
    }
}

void Gameloop::stop() { gameFinished = true; }

// ── Handlers ─────────────────────────────────────────────────────────────

void Gameloop::handleDisconnect(int playerId) {
    if (!game.hasPlayer(playerId))
        return;
    std::cout << "Player " << game.getPlayerName(playerId) << " (id=" << playerId
              << ") disconnected" << std::endl;
    game.removePlayer(playerId);
    clientMonitor.broadcastExcept(
            playerId, std::make_shared<PlayerDisconnectedEvent>(static_cast<uint16_t>(playerId)));
}

void Gameloop::handleUserArrival(int playerId, const std::string& name, const std::string& race,
                                 const std::string& class_) {
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

    // MAP del overworld (mapId=0): copiamos las celdas que viajan por el wire.
    std::vector<MapCellData> cells;
    cells.reserve(map.getCellCount(0));
    for (size_t i = 0; i < map.getCellCount(0); i++) {
        Cell c = map.getCell(i, 0);
        cells.push_back({c.textureId, c.obstacleId, c.safeZone});
    }
    clientMonitor.sendToClient(playerId,
                               std::make_shared<MapEvent>(map.getWidth(0), map.getHeight(0),
                                                          std::move(cells)));

    // Stats iniciales.
    clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));

    // Mandarle un NEW_PLAYER por cada jugador que ya estaba + sus PLAYER_EQUIPPED.
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
    }

    // Avisarles a los demás del recién llegado + su vestimenta.
    const std::string& myName = game.getPlayerName(playerId);
    uint8_t myDir = game.getPlayerDirection(playerId);
    uint8_t mySkin = game.getPlayerSkin(playerId);
    clientMonitor.broadcastExcept(playerId,
                                  std::make_shared<NewPlayerEvent>(static_cast<uint16_t>(playerId),
                                                                   p.x, p.y, myDir, mySkin, myName));
    sendEquipmentSnapshot(playerId, -1);
}

void Gameloop::handleHeadSelected(int /*playerId*/, uint8_t /*headId*/) {
    // TODO(team-gameplay): guardar headSkinId y reenviarlo en NEW_PLAYER cuando
    // se implemente la renderización de cabeza separada del cuerpo.
}

void Gameloop::handleMovement(int playerId, MoveDirection direction) {
    bool success = game.movePlayer(playerId, direction);
    if (success) {
        clientMonitor.sendToClient(
                playerId,
                std::make_shared<OpcodeOnlyEvent>(static_cast<uint8_t>(ServerMsg::MOVE_OK)));
        Position p = game.getPlayerPosition(playerId);
        uint8_t pdir = game.getPlayerDirection(playerId);
        clientMonitor.broadcastExcept(playerId,
                                      std::make_shared<PlayerMovedEvent>(
                                              static_cast<uint16_t>(playerId), p.x, p.y, pdir));
    } else {
        clientMonitor.sendToClient(
                playerId,
                std::make_shared<OpcodeOnlyEvent>(static_cast<uint8_t>(ServerMsg::MOVE_FAIL)));
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
    if (ev->getHit() && ev->getTargetType() == 0 && game.hasPlayer(ev->getTargetId())) {
        clientMonitor.sendToClient(ev->getTargetId(), buildStatsEvent(ev->getTargetId()));
    }
}

void Gameloop::handleCheat(int playerId, uint8_t code) {
    if (!game.hasPlayer(playerId))
        return;
    game.processCheat(playerId, code);
    clientMonitor.sendToClient(playerId, buildStatsEvent(playerId));
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
    auto before = game.getInventorySnapshot(playerId);
    if (!game.pickUpItemAt(playerId))
        return;
    auto after = game.getInventorySnapshot(playerId);
    broadcastInventoryChanges(playerId, before, after, game, clientMonitor);
}

void Gameloop::handleDrop(int playerId, uint8_t invSlot) {
    if (!game.hasPlayer(playerId))
        return;
    auto before = game.getInventorySnapshot(playerId);
    if (!game.dropItem(playerId, invSlot))
        return;
    auto after = game.getInventorySnapshot(playerId);
    broadcastInventoryChanges(playerId, before, after, game, clientMonitor);
}

void Gameloop::handleEquip(int playerId, uint8_t invSlot) {
    if (!game.hasPlayer(playerId))
        return;
    auto before = game.getInventorySnapshot(playerId);
    if (!game.equipOrUseItem(playerId, invSlot))
        return;
    auto after = game.getInventorySnapshot(playerId);
    broadcastInventoryChanges(playerId, before, after, game, clientMonitor);
}

void Gameloop::handleUnequip(int playerId, uint8_t slotType) {
    if (!game.hasPlayer(playerId))
        return;
    auto before = game.getInventorySnapshot(playerId);
    if (!game.unequipSlot(playerId, slotType))
        return;
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
