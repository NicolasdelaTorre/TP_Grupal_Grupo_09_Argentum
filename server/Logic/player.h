#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>
#include <utility>

#include "../../common/position.h"

#include "class_.h"
#include "items.h"
#include "race.h"
#include "stats_definition.h"

#define N 11

struct PlayerData {
    uint32_t experience;
    uint32_t gold;

    Position position;
    uint16_t health;
    uint16_t mana;

    Race::RaceCode race;
    Class_::ClassCode class_;
    uint8_t mapId;
    uint8_t level;
    uint8_t equippedWeapon;
    uint8_t equippedArmor;
    uint8_t equippedHelmet;
    uint8_t equippedShield;
    bool isGhost;

    uint8_t inventory[N];
};

class Player {
private:
    PlayerData data;
    std::string name;
    // dirección actual (valores wire: 3=TOP, 4=BOTTOM, 5=LEFT, 6=RIGHT)
    uint8_t direction = 4;

public:
    explicit Player(const std::string& name, Position position, const std::string& race,
                    const std::string& class_);

    Player(PlayerData data, const std::string& name);

    void move(Position newPosition);
    void setDirection(uint8_t dir);

    const std::string& getName() const;
    Position getPosition() const;
    int16_t getX() const;
    int16_t getY() const;
    uint8_t getDirection() const;
    PlayerData getData() const;
};

#endif
