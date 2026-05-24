#include "game.h"

#include <iostream>
#include <stdexcept>

Game::Game(uint16_t width, uint16_t height): map(width, height) {}

void Game::processCommand(const int playerId, const std::string& command) {
    size_t commandPosition = command.find('.');
    if (commandPosition == std::string::npos) {
        throw std::runtime_error("Game Error: command from client malformed");
    }

    std::string dataType = command.substr(0, commandPosition);

    if (dataType == "user") {
        std::string user = command.substr(commandPosition + 1);
        players.emplace(playerId, Player(user));
        map.addPlayer();
        std::cout << "Hi " << user << std::endl;
    } else if (dataType == "movement") {
        processMovement(playerId, command, commandPosition);
    }
}

void Game::processMovement(const int playerId, const std::string& command,
                            size_t commandPosition) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    size_t positionDirection = command.find('.', commandPosition + 1);
    if (positionDirection == std::string::npos) {
        throw std::runtime_error("Game Error: client command malformed");
    }

    std::string direction =
            command.substr(commandPosition + 1, positionDirection - commandPosition - 1);
    if (map.movePlayer(direction, itPlayer->second.getX(), itPlayer->second.getY())) {
        // algo deberiamos hacer para mostrar algo en el juego
        return;
    }

    itPlayer->second.changePosition(direction);
}
