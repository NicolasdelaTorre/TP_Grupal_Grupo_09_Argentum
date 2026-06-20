#ifndef CREATURE_H
#define CREATURE_H

#include <cstdint>
#include <string>
#include <cmath>
#include <random>
#include <algorithm>

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
    uint8_t agility;
    uint8_t minArmor;
    uint8_t maxArmor;
    uint8_t minShield;
    uint8_t maxShield;
    uint8_t minHelmet;
    uint8_t maxHelmet;

public:
    Creature(uint16_t id, const std::string& name, uint8_t mapId, uint16_t x, uint16_t y, std::string biomeType);

    // Calcula la siguiente posicion hacia el player sin mutar el estado del NPC. El caller decide si moveEntity acepta y recien ahi llama a move().
    Position stalkPlayer(Position playerPosition) const;

    uint16_t receiveDamage(uint16_t damage);

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
