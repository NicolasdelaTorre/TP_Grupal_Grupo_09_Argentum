#ifndef GAME_H
#define GAME_H

#include <string>
#include <unordered_map>
#include <vector>

#include "../../common/position.h"

#include "binary_parser.h"
#include "map.h"
#include "player.h"

class Game {
private:
    Map& map;
    Position playerSpawn;  // posición de spawn que viene del YAML
    std::unordered_map<int, Player> players;
    BinaryParser parser;

    // Encuentra una posición libre para spawnear. Tira excepción si no hay ninguna.
    Position findSpawnPosition() const;

    // True si ningún jugador está parado en pos.
    bool isPositionFree(Position pos) const;

    bool processUser(int playerId, const std::string& user);

    bool processMovement(int playerId, const std::string& direction);

    bool turnPlayer(int playerId, const std::string& direction);

    bool processAttack(int playerId, const std::string& direction);

    bool processHeal(int playerId);

public:
    Game(Map& map, Position playerSpawn);

    bool processCommand(int playerId, const std::string& command);

    Position getPlayerPosition(int playerId) const;

    const std::string& getPlayerName(int playerId) const;

    uint8_t getPlayerDirection(int playerId) const;

    uint16_t getPlayerHealth(int playerId) const;

    uint16_t getPlayerMaxHealth(int playerId) const;

    uint8_t getPlayerLevel(int playerId) const;

    bool hasPlayer(int playerId) const;

    std::vector<int> getPlayerIds() const;

    void updatePlayerData(int playerId);

    void removePlayer(int playerId);

    ~Game();
};

#endif
