#ifndef GAME_H
#define GAME_H

#include <string>
#include <unordered_map>

#include "player.h"
#include "map.h"

class Game {
private:
    Map map;
    std::unordered_map<int, Player> players;

    void processMovement(const int playerId, const std::string& command,
                         size_t commandPosition);

public:
    Game(uint16_t width, uint16_t height);

    void processCommand(const int playerId, const std::string& command);
};

#endif
