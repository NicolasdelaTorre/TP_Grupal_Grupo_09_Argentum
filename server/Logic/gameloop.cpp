#include "gameloop.h"

Gameloop::Gameloop(Queue<std::string>& commands, ClientMonitor& clientQueues, Map& map,
                   ProtocolServer& protocol):
        commands(commands),
        clientQueues(clientQueues),
        gameFinished(false),
        game(map),
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
    std::string checkToSend;

    size_t posId = command.find(':');
    int idPlayer = std::stoi(command.substr(0, posId));

    size_t posCommand = command.find('.', posId);

    std::string cmd = command.substr(posId + 1, posCommand - posId - 1);

    if (cmd == "user") {
        checkToSend += "LOGIN_";
    } else if (cmd == "move") {
        checkToSend += "MOVE_";
    } else {
        std::cout << "Unknown command in gameloop: " << cmd << std::endl;
    }

    if (game.processCommand(idPlayer, command.substr(posId + 1))) {
        checkToSend += "OK";
    } else {
        checkToSend += "FAIL";
    }

    clientQueues.sendToClient(idPlayer, checkToSend);
}

void Gameloop::stop() { gameFinished = true; }
