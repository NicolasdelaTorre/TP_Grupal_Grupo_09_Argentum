#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>

#include "../../common/position.h"

class Player {
private:
    std::string name;
    Position position;

public:
    Player(std::string name, Position position);

    void move(Position newPosition);

    const std::string& getName() const;
    Position getPosition() const;
    int16_t getX() const;
    int16_t getY() const;
};

#endif
