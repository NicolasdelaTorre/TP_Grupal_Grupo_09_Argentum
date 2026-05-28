#include "game.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

Game::Game(Map& map, Position playerSpawn): map(map), playerSpawn(playerSpawn) {}

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
    // Búsqueda en espiral cuadrada desde el spawn del YAML.
    // radius=0 es el spawn, radius=1 sus 8 vecinos, radius=2 los 16 de la siguiente capa, etc.
    int maxRadius = std::max(map.getWidth(), map.getHeight());
    for (int radius = 0; radius < maxRadius; radius++) {
        for (int offsetY = -radius; offsetY <= radius; offsetY++) {
            for (int offsetX = -radius; offsetX <= radius; offsetX++) {
                // Solo la frontera del cuadrado (las interiores ya las chequeamos en radios anteriores).
                if (radius > 0 && std::abs(offsetX) != radius && std::abs(offsetY) != radius)
                    continue;
                Position candidate{static_cast<int16_t>(playerSpawn.x + offsetX),
                                   static_cast<int16_t>(playerSpawn.y + offsetY)};
                if (map.isWalkable(candidate.x, candidate.y) && isPositionFree(candidate)) {
                    return candidate;
                }
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

Position Game::getPlayerPosition(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getPosition();
}

const std::string& Game::getPlayerName(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getName();
}

bool Game::hasPlayer(int playerId) const { return players.find(playerId) != players.end(); }

std::vector<int> Game::getPlayerIds() const {
    std::vector<int> ids;
    ids.reserve(players.size());
    for (const auto& [id, _]: players) {
        ids.push_back(id);
    }
    return ids;
}

void Game::removePlayer(int playerId) { players.erase(playerId); }

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
