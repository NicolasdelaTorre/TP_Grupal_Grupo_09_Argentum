#include "gameloop.h"

Gameloop::Gameloop(Queue<std::string>& commands, ClientMonitor& clientQueues):
        commands(commands), clientQueues(clientQueues), gameFinished(false), game(10, 10) {}

void Gameloop::run() {
    while (!gameFinished) {
        std::string command;
        while (commands.try_pop(command)) {
            size_t posId = command.find(':');
            int idPlayer = std::stoi(command.substr(0, posId));
            std::string cmd = command.substr(posId + 1);

            game.processCommand(idPlayer, cmd);

            if (cmd.substr(0, 7) == "user") {
                clientQueues.sendToClient(idPlayer, "LOGIN_OK");
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void Gameloop::stop() { gameFinished = true; }
