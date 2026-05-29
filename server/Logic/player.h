#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>

#include "race.h"
#include "class_.h"
#include "stats_definition.h"
#include "../../common/position.h"

typedef struct PlayerData {
    Position position;
    uint8_t level;
    Race::RaceCode race;
    Class_::ClassCode class_;
    uint32_t health;
} PlayerData;

class Player {
private:
    // PlayerData data;
    // const std::string& name;

public:
    PlayerData data;
    const std::string& name;
    // dirección actual (valores wire: 3=TOP, 4=BOTTOM, 5=LEFT, 6=RIGHT)
    uint8_t direction = 4;

    explicit Player(const std::string& name, Position position, const std::string& race, const std::string& class_);

    void move(Position newPosition);
    void setDirection(uint8_t dir);

    const std::string& getName() const;
    Position getPosition() const;
    int16_t getX() const;
    int16_t getY() const;
    uint8_t getDirection() const;
};

#endif
