#include "game.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

Game::Game(Map& map, Position playerSpawn):
        map(map), playerSpawn(playerSpawn), parser(BinaryParser()) {}

bool Game::processCommand(int playerId, const std::string& command) {
    size_t commandPosition = command.find('.');
    if (commandPosition == std::string::npos) {
        throw std::runtime_error("Game Error: command from client malformed");
    }

    std::string dataType = command.substr(0, commandPosition);

    if (dataType == "user") {
        std::string user = command.substr(commandPosition + 1);
        return processUser(playerId, user);
    } else if (dataType == "movement") {
        std::string direction = command.substr(commandPosition + 1);
        return processMovement(playerId, direction);
    } else if (dataType == "turn") {
        std::string direction = command.substr(commandPosition + 1);
        return turnPlayer(playerId, direction);
    } else if (dataType == "attack") {
        // Formato: "attack"
        std::string direction = command.substr(commandPosition + 1);
        return processAttack(playerId, direction);
    }

    return false;
}

bool Game::processUser(int playerId, const std::string& user) {
    Position spawn;

    if (!parser.checkPlayerExists(user)) {
        // Create new player.
        spawn = findSpawnPosition();
        players.emplace(playerId, Player(user, spawn, "Elf", "Mage"));
        parser.savePlayerData(user, players.at(playerId).getData());
    } else {
        // Restore existing player.
        players.emplace(playerId, Player(parser.loadPlayerData(user), user));
        spawn = players.at(playerId).getPosition();
    }

    map.placePlayer(playerId, spawn.x, spawn.y);

    std::cout << "Hi " << user << " spawned at (" << spawn.x << ", " << spawn.y << ")" << std::endl;

    return true;
}

bool Game::turnPlayer(int playerId, const std::string& direction) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        return false;
    }
    uint8_t newDir;
    if (direction == "top")
        newDir = 3;
    else if (direction == "bottom")
        newDir = 4;
    else if (direction == "left")
        newDir = 5;
    else if (direction == "right")
        newDir = 6;
    else
        return false;

    itPlayer->second.setDirection(newDir);
    return true;
}

Position Game::findSpawnPosition() const {
    // Búsqueda en espiral cuadrada desde el spawn del YAML.
    // radius=0 es el spawn, radius=1 sus 8 vecinos, radius=2 los 16 de la siguiente capa, etc.
    int maxRadius = std::max(map.getWidth(), map.getHeight());
    for (int radius = 0; radius < maxRadius; radius++) {
        for (int offsetY = -radius; offsetY <= radius; offsetY++) {
            for (int offsetX = -radius; offsetX <= radius; offsetX++) {
                // Solo la frontera del cuadrado (las interiores ya las chequeamos en radios
                // anteriores).
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

uint8_t Game::getPlayerDirection(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getDirection();
}

uint16_t Game::getPlayerHealth(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getData().health;
}

uint16_t Game::getPlayerMaxHealth(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getData().maxHealth;
}

uint8_t Game::getPlayerLevel(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }
    return it->second.getData().level;
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

void Game::updatePlayerData(int playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    parser.updatePlayerData(it->second.getName(), it->second.getData());
}

void Game::removePlayer(int playerId) {
    updatePlayerData(playerId);
    players.erase(playerId);
}

bool Game::processMovement(int playerId, const std::string& direction) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    Player& player = itPlayer->second;
    Position next = player.getPosition();
    uint8_t newDir = 4;

    if (direction == "top") {
        next.y -= 1;
        newDir = 3;
    } else if (direction == "bottom") {
        next.y += 1;
        newDir = 4;
    } else if (direction == "left") {
        next.x -= 1;
        newDir = 5;
    } else if (direction == "right") {
        next.x += 1;
        newDir = 6;
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
    player.setDirection(newDir);
    return true;
}

bool Game::processAttack(int playerId, const std::string& direction) {
    auto itPlayer = players.find(playerId);
    if (itPlayer == players.end()) {
        throw std::runtime_error("Game Error: player not found");
    }

    if (!itPlayer->second.isEquipped() || !itPlayer->second.isAlive()) {
        return false;
    }

    uint8_t entityId = map.isEntityInSight(itPlayer->second.getX(), itPlayer->second.getY(),
                                           direction, itPlayer->second.hasLongDistanceWeapon());

    if (entityId == 0)
        return false;

    auto itTarget = players.find(entityId);
    if (itTarget == players.end()) {
        throw std::runtime_error("Game Error: player in sight not found");
    }

    itTarget->second.receiveDamage(itPlayer->second.dealDamage());

    return true;
}

Game::~Game() {
    for (const auto& [id, _]: players) {
        updatePlayerData(id);
    }
}
