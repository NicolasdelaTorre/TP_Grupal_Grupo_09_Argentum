#ifndef GAME_H
#define GAME_H

#include <string>
#include <unordered_map>

#include "map.h"
#include "player.h"

class Game {
private:
    Map& map;
    std::unordered_map<int, Player> players;

    bool processMovement(const int playerId, const std::string& direction);

public:
    explicit Game(Map& map);

    bool processCommand(const int playerId, const std::string& command);
};

#endif
