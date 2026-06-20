#include "creature.h"

#include <cstdlib>
#include <stdexcept>

#include "../catalog/creature_catalog.h"

Creature::Creature(uint16_t id, const std::string& name, uint8_t mapId, uint16_t x, uint16_t y, std::string biomeType) : NPC(id, name, x, y, mapId), isAlive(true), biomeType(biomeType) {
    const auto& sp = CreatureCatalog::instance().getSpawn();
    if (mapId == 0) {
        uint8_t span = sp.overworldLevelMax - sp.overworldLevelMin + 1;
        level = sp.overworldLevelMin + rand() % span;
    } else {
        uint8_t span = sp.dungeonLevelMax - sp.dungeonLevelMin + 1;
        level = sp.dungeonLevelMin + rand() % span;
    }

    const CreatureAttributes& attrs = CreatureCatalog::instance().findByName(name);
    maxHealth = attrs.maxHealth * level;
    health = maxHealth;
    damage = attrs.damage * level;
    agility = attrs.agility;
    minArmor = attrs.minArmor;
    maxArmor = attrs.maxArmor;
    minShield = attrs.minShield;
    maxShield = attrs.maxShield;
    minHelmet = attrs.minHelmet;
    maxHelmet = attrs.maxHelmet;
}

Position Creature::stalkPlayer(Position playerPosition) const {
    if (playerPosition.x == -1) {
        return {-1, -1};
    }

    int16_t dx = playerPosition.x - position.x;
    int16_t dy = playerPosition.y - position.y;

    return {static_cast<int16_t>(position.x + (dx > 0) - (dx < 0)),
            static_cast<int16_t>(position.y + (dy > 0) - (dy < 0))};
}

uint16_t Creature::receiveDamage(uint16_t damage) {
    // Try to Evade
    std::random_device rd;  
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    if (std::pow(dis(gen), static_cast<double>(agility)) < 0.001) {
        return 0;
    }

    // Calculate Creature Defense
    std::uniform_int_distribution<int> dis1(minArmor, maxArmor);
    uint16_t armorDefense = static_cast<uint16_t>(dis1(gen));
    std::uniform_int_distribution<int> dis2(minShield, maxShield);
    uint16_t shieldDefense = static_cast<uint16_t>(dis2(gen));
    std::uniform_int_distribution<int> dis3(minHelmet, maxHelmet);
    uint16_t helmetDefense = static_cast<uint16_t>(dis3(gen));
    uint16_t defense = armorDefense + shieldDefense + helmetDefense;

    // Decrease health
    if (defense >= damage) {
        return 0;
    }

    if ((damage - defense) >= health) {
        health = 0;
        isAlive = false;
        return health;
    }

    health -= (damage - defense);
    return (damage - defense);
}

void Creature::resurrect() {
    health = maxHealth;
    isAlive = true;
}

bool Creature::isDead() {
    return !isAlive;
}

Position Creature::getPosition() const {
    return position;
}

uint16_t Creature::getDamage() const { return damage; }

uint16_t Creature::getMaxHealth() const { return maxHealth; }

uint8_t Creature::getLevel() const { return level; }

uint8_t Creature::getMapId() const {
    return mapId;
}

std::string Creature::getBiomeType() const {
    return biomeType;
}
