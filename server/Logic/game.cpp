#include "game.h"

#include <iostream>
#include <stdexcept>

Game::Game(uint16_t width, uint16_t height): map(width, height) {}

bool Game::processCommand(const int playerId, const std::string& command) {
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
        return true;
    } else if (dataType == "movement") {
        std::string direction = command.substr(commandPosition + 1);
        return processMovement(playerId, direction);
    }

    return false;
}

bool Game::processMovement(const int playerId, const std::string& direction) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    if (!map.movePlayer(direction, itPlayer->second.getX(), itPlayer->second.getY())) {
        // algo deberiamos hacer para mostrar algo en el juego
        std::cout << "Player can't move in that direction" << std::endl;
        return false;
    }

    itPlayer->second.changePosition(direction);
    return true;
}
