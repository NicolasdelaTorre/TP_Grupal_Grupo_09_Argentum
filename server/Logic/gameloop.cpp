#include "gameloop.h"

Gameloop::Gameloop(Queue<std::string>& commands, ClientMonitor& clientQueues, Map& map,
                   ProtocolServer& protocol, Position playerSpawn):
        commands(commands),
        clientQueues(clientQueues),
        gameFinished(false),
        game(map, playerSpawn),
        protocol(protocol) {}

void Gameloop::run() {
    while (!gameFinished) {
        std::string command;
        while (commands.try_pop(command)) {
            processCommand(command);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

void Gameloop::processCommand(const std::string& command) {
    size_t posId = command.find(':');
    int idPlayer = std::stoi(command.substr(0, posId));

    size_t posCommand = command.find('.', posId);
    std::string cmd = command.substr(posId + 1, posCommand - posId - 1);

    // El Receiver lo arma cuando se cierra el socket
    if (cmd == "disconnect") {
        if (game.hasPlayer(idPlayer)) {
            game.removePlayer(idPlayer);
            std::string msg = "PLAYER_DISCONNECTED:" + std::to_string(idPlayer);
            clientQueues.broadcastExcept(idPlayer, msg);
        }
        return;
    }

    // "skin" llega después del char creation. No la procesa el Game (no cambia
    // estado del mundo, por ahora), solo gatilla la finalización del login.
    if (cmd == "skin") {
        std::string skinId = command.substr(posCommand + 1);
        if (game.hasPlayer(idPlayer)) {
            finalizePlayerLogin(idPlayer, skinId);
        }
        return;
    }

    // "attack.<type>.<id>" — ataque a un target específico. El server valida
    // arma equipada y alcance (rango/adyacencia). Broadcast del resultado a todos.
    if (cmd == "attack") {
        if (game.hasPlayer(idPlayer)) {
            std::string payload = command.substr(posCommand + 1);
            size_t dot = payload.find('.');
            if (dot != std::string::npos) {
                uint8_t targetType = static_cast<uint8_t>(std::stoi(payload.substr(0, dot)));
                uint16_t targetId = static_cast<uint16_t>(std::stoi(payload.substr(dot + 1)));
                AttackResult r = game.processAttack(idPlayer, targetType, targetId);
                std::cout << "ATTACK from player=" << idPlayer
                          << " ttype=" << (int)targetType << " tid=" << targetId
                          << " performed=" << r.performed << " hit=" << r.hit
                          << " dmg=" << r.damage << std::endl;
                if (r.performed) {
                    std::string msg = "ATTACK_RESULT:" + std::to_string(r.attackerId) + ":" +
                                      std::to_string(static_cast<int>(r.targetType)) + ":" +
                                      std::to_string(r.targetId) + ":" +
                                      std::to_string(r.damage) + ":" +
                                      std::to_string(r.hit ? 1 : 0);
                    clientQueues.broadcast(msg);
                    if (r.hit && r.targetType == 0 && game.hasPlayer(r.targetId)) {
                        clientQueues.sendToClient(r.targetId, buildStatsMessage(r.targetId));
                    }
                }
            }
        }
        return;
    }

    // "head.<id>" — cabeza elegida en char creation. TODO(team-gameplay):
    // guardar headSkinId en el Player y reenviarlo en NEW_PLAYER cuando se
    // implemente la renderización de cabeza separada del cuerpo. Por ahora
    // solo lo aceptamos para que el cliente no rompa el flujo de login.
    if (cmd == "head") {
        return;
    }

    // "cheat.<code>" — aplica un cheat y reenvía los stats actualizados.
    if (cmd == "cheat") {
        if (game.hasPlayer(idPlayer)) {
            uint8_t code = static_cast<uint8_t>(std::stoi(command.substr(posCommand + 1)));
            game.processCheat(idPlayer, code);
            // Los stats pueden haber cambiado (vida=0 en suicide, gold/exp etc).
            // Reenviamos para que el HUD del cliente se actualice.
            clientQueues.sendToClient(idPlayer, buildStatsMessage(idPlayer));
        }
        return;
    }

    // Comandos de inventario: pickup / drop / equip / unequip.
    // Todos mandan INVENTORY_UPDATE al dueño; si cambió algún slot equipado,
    // además broadcast PLAYER_EQUIPPED para que los demás vean la vestimenta.
    // Format: cmd:
    if (cmd == "pickup" || cmd == "drop" || cmd == "equip" || cmd == "unequip") {
        if (!game.hasPlayer(idPlayer)) {
            return;
        }
        auto before = game.getInventorySnapshot(idPlayer);
        bool ok = false;
        if (cmd == "pickup") {
            ok = game.pickUpItemAt(idPlayer);
        } else if (cmd == "drop") {
            uint8_t slot = static_cast<uint8_t>(std::stoi(command.substr(posCommand + 1)));
            ok = game.dropItem(idPlayer, slot);
        } else if (cmd == "equip") {
            uint8_t slot = static_cast<uint8_t>(std::stoi(command.substr(posCommand + 1)));
            ok = game.equipOrUseItem(idPlayer, slot);
        } else {  // unequip
            uint8_t slotType = static_cast<uint8_t>(std::stoi(command.substr(posCommand + 1)));
            ok = game.unequipSlot(idPlayer, slotType);
        }
        if (!ok) {
            return;
        }
        auto after = game.getInventorySnapshot(idPlayer);

        // INVENTORY_UPDATE al dueño (snapshot completo).
        std::string msg = "INVENTORY:" + std::to_string(after.items.size());
        for (uint8_t id: after.items) {
            msg += ":" + std::to_string(static_cast<int>(id));
        }
        msg += ":" + std::to_string(static_cast<int>(after.equippedWeapon));
        msg += ":" + std::to_string(static_cast<int>(after.equippedArmor));
        msg += ":" + std::to_string(static_cast<int>(after.equippedHelmet));
        msg += ":" + std::to_string(static_cast<int>(after.equippedShield));
        clientQueues.sendToClient(idPlayer, msg);

        // PLAYER_EQUIPPED broadcast por cada slot equipado que cambió.
        const uint8_t beforeSlots[4] = {before.equippedWeapon, before.equippedArmor,
                                        before.equippedHelmet, before.equippedShield};
        const uint8_t afterSlots[4] = {after.equippedWeapon, after.equippedArmor,
                                       after.equippedHelmet, after.equippedShield};
        // Al dueño NO le mandamos PLAYER_EQUIPPED: ya recibió INVENTORY_UPDATE
        // con todos sus slots equipados. Solo notificamos a los demás.
        for (uint8_t s = 0; s < 4; s++) {
            if (beforeSlots[s] != afterSlots[s]) {
                std::string eqMsg = "PLAYER_EQUIPPED:" + std::to_string(idPlayer) + ":" +
                                    std::to_string(static_cast<int>(s)) + ":" +
                                    std::to_string(static_cast<int>(afterSlots[s]));
                clientQueues.broadcastExcept(idPlayer, eqMsg);
            }
        }
        return;
    }

    bool success = game.processCommand(idPlayer, command.substr(posId + 1));

    if (cmd == "user") {
        // El usuario se acaba de loguear. Le mandamos FIRST_LOGIN para que
        // muestre la pantalla de selección de personaje; los NEW_PLAYER y el
        // MAP se mandan cuando llegue el "skin".
        clientQueues.sendToClient(idPlayer, success ? "FIRST_LOGIN" : "LOGIN_FAIL");
    } else if (cmd == "movement") {
        if (success) {
            clientQueues.sendToClient(idPlayer, "MOVE_OK");
            // Avisar a los demás del movimiento.
            Position p = game.getPlayerPosition(idPlayer);
            uint8_t dir = game.getPlayerDirection(idPlayer);
            std::string moveMsg = "PLAYER_MOVED:" + std::to_string(idPlayer) + ":" +
                                  std::to_string(p.x) + ":" + std::to_string(p.y) + ":" +
                                  std::to_string(dir);
            clientQueues.broadcastExcept(idPlayer, moveMsg);
        } else {
            clientQueues.sendToClient(idPlayer, "MOVE_FAIL");
        }
    } else if (cmd == "turn") {
        // Gira sin moverse: misma posición, nueva dirección.
        if (success) {
            Position p = game.getPlayerPosition(idPlayer);
            uint8_t dir = game.getPlayerDirection(idPlayer);
            std::string turnMsg = "PLAYER_MOVED:" + std::to_string(idPlayer) + ":" +
                                  std::to_string(p.x) + ":" + std::to_string(p.y) + ":" +
                                  std::to_string(dir);
            clientQueues.broadcastExcept(idPlayer, turnMsg);
        }
    } else {
        std::cout << "Unknown command in gameloop: " << cmd << std::endl;
    }
}

void Gameloop::finalizePlayerLogin(int idPlayer, const std::string& skinId) {
    Position p = game.getPlayerPosition(idPlayer);
    game.setSkin(idPlayer, skinId);
    std::string loginMsg = "LOGIN_OK:" + std::to_string(p.x) + ":" + std::to_string(p.y);
    clientQueues.sendToClient(idPlayer, loginMsg);
    clientQueues.sendToClient(idPlayer, "MAP");

    // Stats iniciales (hp/mana/gold/exp/level — snapshot completo).
    clientQueues.sendToClient(idPlayer, buildStatsMessage(idPlayer));

    // Mandarle un NEW_PLAYER por cada jugador que ya estaba + sus PLAYER_EQUIPPED.
    for (int otherId: game.getPlayerIds()) {
        if (otherId == idPlayer)
            continue;
        Position op = game.getPlayerPosition(otherId);
        const std::string& oname = game.getPlayerName(otherId);
        uint8_t odir = game.getPlayerDirection(otherId);
        uint8_t oskin = game.getPlayerSkin(otherId);
        std::string np = "NEW_PLAYER:" + std::to_string(otherId) + ":" + std::to_string(op.x) +
                         ":" + std::to_string(op.y) + ":" + std::to_string(odir) + ":" +
                         std::to_string(oskin) + ":" + oname;
        clientQueues.sendToClient(idPlayer, np);
        sendEquipmentSnapshot(otherId, idPlayer);
    }

    // Avisarles a los demás del recién llegado + su vestimenta.
    const std::string& myName = game.getPlayerName(idPlayer);
    uint8_t myDir = game.getPlayerDirection(idPlayer);
    uint8_t mySkin = game.getPlayerSkin(idPlayer);
    std::string broadcastMsg = "NEW_PLAYER:" + std::to_string(idPlayer) + ":" +
                               std::to_string(p.x) + ":" + std::to_string(p.y) + ":" +
                               std::to_string(myDir) + ":" + std::to_string(mySkin) + ":" + myName;
    clientQueues.broadcastExcept(idPlayer, broadcastMsg);
    sendEquipmentSnapshot(idPlayer, -1);  // -1 = broadcast a todos menos a él mismo
}

void Gameloop::sendEquipmentSnapshot(int idPlayer, int recipientId) {
    auto snap = game.getInventorySnapshot(idPlayer);
    const uint8_t slots[4] = {snap.equippedWeapon, snap.equippedArmor,
                              snap.equippedHelmet, snap.equippedShield};
    for (uint8_t s = 0; s < 4; s++) {
        if (slots[s] == 0)
            continue;  // nada equipado, no mando
        std::string eqMsg = "PLAYER_EQUIPPED:" + std::to_string(idPlayer) + ":" +
                            std::to_string(static_cast<int>(s)) + ":" +
                            std::to_string(static_cast<int>(slots[s]));
        if (recipientId < 0) {
            clientQueues.broadcastExcept(idPlayer, eqMsg);
        } else {
            clientQueues.sendToClient(recipientId, eqMsg);
        }
    }
}

std::string Gameloop::buildStatsMessage(int idPlayer) {
    uint16_t hp = game.getPlayerHealth(idPlayer);
    uint16_t maxHp = game.getPlayerMaxHealth(idPlayer);
    uint16_t mana = game.getPlayerMana(idPlayer);
    uint16_t maxMana = game.getPlayerMaxMana(idPlayer);
    uint32_t gold = game.getPlayerGold(idPlayer);
    uint32_t exp = game.getPlayerExperience(idPlayer);
    uint32_t nextLvlExp = game.getPlayerNextLevelExp(idPlayer);
    uint8_t level = game.getPlayerLevel(idPlayer);
    return "STATS:" + std::to_string(hp) + ":" + std::to_string(maxHp) + ":" +
           std::to_string(mana) + ":" + std::to_string(maxMana) + ":" +
           std::to_string(gold) + ":" + std::to_string(exp) + ":" +
           std::to_string(nextLvlExp) + ":" + std::to_string(static_cast<int>(level));
}

void Gameloop::stop() { gameFinished = true; }
