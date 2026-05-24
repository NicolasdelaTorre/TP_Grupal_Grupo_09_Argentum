#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>

typedef struct Position {
    int16_t x;
    int16_t y;
} Position;

typedef struct PlayerData {
    const std::string& name;
    Position position;
} PlayerData;

class Player {
private:
    PlayerData data;

public:
    explicit Player(const std::string& name);

    void changePosition(const std::string& direction);

    int16_t getX();

    int16_t getY();
};

#endif
