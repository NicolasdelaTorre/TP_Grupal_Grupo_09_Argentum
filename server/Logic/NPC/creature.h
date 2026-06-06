#ifndef CREATURE_H
#define CREATURE_H

#include <cstdint>
#include <string>

#include "../../common/position.h"
#include "../map.h"

class Creature {
    private:
        uint8_t id;
        const std::string& name;
        uint8_t level;
        uint16_t health;
        uint16_t maxHealth;
        uint16_t damage;
        Position position;
        Map& map;

    public:
        Creature(const std::string& name, uint8_t mapId, uint16_t x, uint16_t y, Map& map);

        void stalkPlayer();

        uint16_t attackPlayer();

        void receiveDamage(uint16_t damage);
};

#endif
