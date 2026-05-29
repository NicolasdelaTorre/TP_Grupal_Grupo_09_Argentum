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

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
        if (game.hasPlayer(idPlayer)) {
            finalizePlayerLogin(idPlayer);
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
            std::string moveMsg = "PLAYER_MOVED:" + std::to_string(idPlayer) + ":" +
                                  std::to_string(p.x) + ":" + std::to_string(p.y);
            clientQueues.broadcastExcept(idPlayer, moveMsg);
        } else {
            clientQueues.sendToClient(idPlayer, "MOVE_FAIL");
        }
    } else {
        std::cout << "Unknown command in gameloop: " << cmd << std::endl;
    }
}

void Gameloop::finalizePlayerLogin(int idPlayer) {
    Position p = game.getPlayerPosition(idPlayer);
    std::string loginMsg = "LOGIN_OK:" + std::to_string(p.x) + ":" + std::to_string(p.y);
    clientQueues.sendToClient(idPlayer, loginMsg);
    clientQueues.sendToClient(idPlayer, "MAP");

    // Mandarle un NEW_PLAYER por cada jugador que ya estaba.
    for (int otherId: game.getPlayerIds()) {
        if (otherId == idPlayer)
            continue;
        Position op = game.getPlayerPosition(otherId);
        const std::string& oname = game.getPlayerName(otherId);
        std::string np = "NEW_PLAYER:" + std::to_string(otherId) + ":" +
                         std::to_string(op.x) + ":" + std::to_string(op.y) + ":" + oname;
        clientQueues.sendToClient(idPlayer, np);
    }

    // Avisarles a los demás del recién llegado.
    const std::string& myName = game.getPlayerName(idPlayer);
    std::string broadcastMsg = "NEW_PLAYER:" + std::to_string(idPlayer) + ":" +
                               std::to_string(p.x) + ":" + std::to_string(p.y) + ":" + myName;
    clientQueues.broadcastExcept(idPlayer, broadcastMsg);
}

void Gameloop::stop() { gameFinished = true; }
