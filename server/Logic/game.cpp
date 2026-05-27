#include "game.h"

#include <iostream>
#include <stdexcept>

Game::Game(Map& map): map(map) {}

bool Game::processCommand(int playerId, const std::string& command) {
    size_t commandPosition = command.find('.');
    if (commandPosition == std::string::npos) {
        throw std::runtime_error("Game Error: command from client malformed");
    }

    std::string dataType = command.substr(0, commandPosition);

    if (dataType == "user") {
        std::string user = command.substr(commandPosition + 1);
        Position spawn = findSpawnPosition();
        players.emplace(playerId, Player(user, spawn));
        std::cout << "Hi " << user << " spawned at (" << spawn.x << ", " << spawn.y << ")"
                  << std::endl;
        return true;
    } else if (dataType == "movement") {
        std::string direction = command.substr(commandPosition + 1);
        return processMovement(playerId, direction);
    }

    return false;
}

Position Game::findSpawnPosition() const {
    // Primer intento: centro del mapa
    int16_t centerX = static_cast<int16_t>(map.getWidth() / 2);
    int16_t centerY = static_cast<int16_t>(map.getHeight() / 2);
    Position center{centerX, centerY};

    if (map.isWalkable(center.x, center.y) && isPositionFree(center)) {
        return center;
    }

    // Fallback: primera celda caminable y desocupada
    for (uint16_t y = 0; y < map.getHeight(); y++) {
        for (uint16_t x = 0; x < map.getWidth(); x++) {
            Position pos{static_cast<int16_t>(x), static_cast<int16_t>(y)};
            if (map.isWalkable(pos.x, pos.y) && isPositionFree(pos)) {
                return pos;
            }
        }
    }

    throw std::runtime_error("Game Error: no free cell available for new player");
}

bool Game::isPositionFree(Position pos) const {
    for (const auto& [id, player]: players) {
        if (player.getX() == pos.x && player.getY() == pos.y) {
            return false;
        }
    }
    return true;
}

bool Game::processMovement(int playerId, const std::string& direction) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    Player& player = itPlayer->second;
    Position next = player.getPosition();

    if (direction == "top") {
        next.y -= 1;
    } else if (direction == "bottom") {
        next.y += 1;
    } else if (direction == "left") {
        next.x -= 1;
    } else if (direction == "right") {
        next.x += 1;
    } else {
        return false;
    }

    if (!map.isWalkable(next.x, next.y)) {
        std::cout << "Player can't move in that direction (blocked)" << std::endl;
        return false;
    }

    if (!isPositionFree(next)) {
        std::cout << "Player can't move in that direction (occupied by another player)"
                  << std::endl;
        return false;
    }

    player.move(next);
    return true;
}
