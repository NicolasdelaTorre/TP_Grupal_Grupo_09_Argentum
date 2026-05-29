#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>

#include "../../common/position.h"

class Player {
private:
    std::string name;
    Position position;
    // dirección actual (valores wire: 3=TOP, 4=BOTTOM, 5=LEFT, 6=RIGHT)
    uint8_t direction = 4;

public:
    Player(std::string name, Position position);

    void move(Position newPosition);
    void setDirection(uint8_t dir);

    const std::string& getName() const;
    Position getPosition() const;
    int16_t getX() const;
    int16_t getY() const;
    uint8_t getDirection() const;
};

#endif
