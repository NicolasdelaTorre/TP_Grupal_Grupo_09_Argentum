#ifndef BINARY_PARSER_H
#define BINARY_PARSER_H

#include <string>

#include "player.h"

class BinaryParser {
private:
    uint32_t lastOffset;

public:
    BinaryParser();

    bool checkPlayerExists(const std::string& name);

    void savePlayerData(const std::string& name, PlayerData data);

    PlayerData loadPlayerData(const std::string& name);

    void updatePlayerData(const std::string& name, PlayerData data);
};

#endif
