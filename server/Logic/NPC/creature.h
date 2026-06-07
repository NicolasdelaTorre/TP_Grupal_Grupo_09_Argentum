#ifndef CREATURE_H
#define CREATURE_H

#include <cstdint>
#include <string>

#include "../../common/position.h"
#include "npc.h"

class Creature : public NPC {
private:
    uint8_t level;
    uint16_t health;
    uint16_t maxHealth;
    uint16_t damage;

public:
    Creature(uint16_t id, const std::string& name, uint8_t mapId, uint16_t x, uint16_t y);

    void stalkPlayer(Position playerPosition);

    uint16_t attackPlayer(uint8_t playerId);

    void receiveDamage(uint16_t damage);
};

#endif
