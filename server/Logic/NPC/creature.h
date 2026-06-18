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
    bool isAlive;
    std::string biomeType;

public:
    Creature(uint16_t id, const std::string& name, uint8_t mapId, uint16_t x, uint16_t y, std::string biomeType);

    // Calcula la siguiente posicion hacia el player sin mutar el estado del NPC. El caller decide si moveEntity acepta y recien ahi llama a move().
    Position stalkPlayer(Position playerPosition) const;

    void receiveDamage(uint16_t damage);

    void resurrect();

    bool isDead();

    Position getPosition() const;

    uint16_t getDamage() const;

    uint16_t getMaxHealth() const;

    uint8_t getLevel() const;

    uint8_t getMapId() const;

    std::string getBiomeType() const;
};

#endif
