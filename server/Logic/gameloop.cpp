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

    // "attack.<dir>" — resuelve el ataque y broadcast del resultado a todos.
    if (cmd == "attack") {
        if (game.hasPlayer(idPlayer)) {
            std::string direction = command.substr(posCommand + 1);
            AttackResult r = game.processAttack(idPlayer, direction);
            std::cout << "ATTACK from player=" << idPlayer << " dir=" << direction
                      << " performed=" << r.performed << " hit=" << r.hit
                      << " target=" << r.targetId << " dmg=" << r.damage << std::endl;
            if (r.performed) {
                std::string msg = "ATTACK_RESULT:" + std::to_string(r.attackerId) + ":" +
                                  std::to_string(static_cast<int>(r.targetType)) + ":" +
                                  std::to_string(r.targetId) + ":" + std::to_string(r.damage) +
                                  ":" + std::to_string(r.hit ? 1 : 0);
                clientQueues.broadcast(msg);
                // El target sufrió daño → mandarle sus stats actualizados.
                if (r.hit && r.targetType == 0 && game.hasPlayer(r.targetId)) {
                    uint16_t hp = game.getPlayerHealth(r.targetId);
                    uint16_t maxHp = game.getPlayerMaxHealth(r.targetId);
                    uint8_t lvl = game.getPlayerLevel(r.targetId);
                    std::string stats = "STATS:" + std::to_string(hp) + ":" +
                                        std::to_string(maxHp) + ":" +
                                        std::to_string(static_cast<int>(lvl));
                    clientQueues.sendToClient(r.targetId, stats);
                }
            }
        }
        return;
    }

    // "targeted_attack.<type>.<id>" — ataque a un target específico (ranged/magia).
    if (cmd == "targeted_attack") {
        if (game.hasPlayer(idPlayer)) {
            std::string payload = command.substr(posCommand + 1);
            size_t dot = payload.find('.');
            if (dot != std::string::npos) {
                uint8_t targetType = static_cast<uint8_t>(std::stoi(payload.substr(0, dot)));
                uint16_t targetId = static_cast<uint16_t>(std::stoi(payload.substr(dot + 1)));
                AttackResult r = game.processTargetedAttack(idPlayer, targetType, targetId);
                std::cout << "TARGETED_ATTACK from player=" << idPlayer
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
                        uint16_t hp = game.getPlayerHealth(r.targetId);
                        uint16_t maxHp = game.getPlayerMaxHealth(r.targetId);
                        uint8_t lvl = game.getPlayerLevel(r.targetId);
                        std::string stats = "STATS:" + std::to_string(hp) + ":" +
                                            std::to_string(maxHp) + ":" +
                                            std::to_string(static_cast<int>(lvl));
                        clientQueues.sendToClient(r.targetId, stats);
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
            uint16_t hp = game.getPlayerHealth(idPlayer);
            uint16_t maxHp = game.getPlayerMaxHealth(idPlayer);
            uint8_t level = game.getPlayerLevel(idPlayer);
            std::string statsMsg = "STATS:" + std::to_string(hp) + ":" + std::to_string(maxHp) +
                                   ":" + std::to_string(static_cast<int>(level));
            clientQueues.sendToClient(idPlayer, statsMsg);
        }
        return;
    }

    // Comandos de inventario: pickup / drop / equip / unequip.
    // Todos terminan mandando INVENTORY_UPDATE sólo al dueño.
    if (cmd == "pickup" || cmd == "drop" || cmd == "equip" || cmd == "unequip") {
        if (!game.hasPlayer(idPlayer)) {
            return;
        }
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
        // Armamos el INVENTORY_UPDATE como string interno.
        auto snap = game.getInventorySnapshot(idPlayer);
        std::string msg = "INVENTORY:" + std::to_string(snap.items.size());
        for (uint8_t id: snap.items) {
            msg += ":" + std::to_string(static_cast<int>(id));
        }
        msg += ":" + std::to_string(static_cast<int>(snap.equippedWeapon));
        msg += ":" + std::to_string(static_cast<int>(snap.equippedArmor));
        msg += ":" + std::to_string(static_cast<int>(snap.equippedHelmet));
        msg += ":" + std::to_string(static_cast<int>(snap.equippedShield));
        clientQueues.sendToClient(idPlayer, msg);
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

    // Stats iniciales (vida actual / vida máxima / nivel)
    uint32_t hp = game.getPlayerHealth(idPlayer);
    uint32_t maxHp = game.getPlayerMaxHealth(idPlayer);
    uint8_t level = game.getPlayerLevel(idPlayer);
    std::string statsMsg = "STATS:" + std::to_string(hp) + ":" + std::to_string(maxHp) + ":" +
                           std::to_string(static_cast<int>(level));
    clientQueues.sendToClient(idPlayer, statsMsg);

    // Mandarle un NEW_PLAYER por cada jugador que ya estaba.
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
    }

    // Avisarles a los demás del recién llegado.
    const std::string& myName = game.getPlayerName(idPlayer);
    uint8_t myDir = game.getPlayerDirection(idPlayer);
    uint8_t mySkin = game.getPlayerSkin(idPlayer);
    std::string broadcastMsg = "NEW_PLAYER:" + std::to_string(idPlayer) + ":" +
                               std::to_string(p.x) + ":" + std::to_string(p.y) + ":" +
                               std::to_string(myDir) + ":" + std::to_string(mySkin) + ":" + myName;
    clientQueues.broadcastExcept(idPlayer, broadcastMsg);
}

void Gameloop::stop() { gameFinished = true; }
