#include "creature.h"

#include <ctime>

#include "../toml.hpp"
#include "../stats_definition.h"
#include <iostream>

<<<<<<< HEAD
Creature::Creature(uint16_t id, const std::string& name, uint8_t mapId, uint16_t x, uint16_t y) : NPC(id, name, x, y, mapId), isAlive(true) {
    // Rangos de level por zona vienen del TOML.
=======
Creature::Creature(uint16_t id, const std::string& name, uint8_t mapId, uint16_t x, uint16_t y, std::string biomeType) : NPC(id, name, x, y, mapId), isAlive(true), biomeType(biomeType) {
    // Set level
>>>>>>> ca8bd0d (fix: solución a errores de algunos npcs)
    srand(time(nullptr));
    CreatureSpawnConfig sp = StatsDefinition().getSpawnConfig();
    if (mapId == 0) {
        uint8_t span = sp.overworldLevelMax - sp.overworldLevelMin + 1;
        level = sp.overworldLevelMin + rand() % span;
    } else {
        uint8_t span = sp.dungeonLevelMax - sp.dungeonLevelMin + 1;
        level = sp.dungeonLevelMin + rand() % span;
    }

    const toml::value config = toml::parse("server/Logic/NPC/npc.toml");

    const auto npcs = toml::find<std::vector<toml::value>>(config, "npc");

    for (const auto& npc: npcs) {
        if (toml::find<std::string>(npc, "name") == name) {
            maxHealth = toml::find<uint16_t>(npc, "maxHealth") * level;
            health = maxHealth;
            damage = toml::find<uint16_t>(npc, "damage") * level;
            break;
        }
    }

    std::cout << "Spawned creature " << name << " with level " << level << ", health " << health
              << " and damage " << damage << " in position (" << position.x << ", " << position.y << ")" << std::endl;
}

Position Creature::stalkPlayer(Position playerPosition) {
    // No player in sight
    if (playerPosition.x == -1) {
        return {-1, -1};
    }

    int16_t dx = playerPosition.x - position.x;
    int16_t dy = playerPosition.y - position.y;

    position.x += (dx > 0) - (dx < 0);
    position.y += (dy > 0) - (dy < 0);

    return position;
}

void Creature::receiveDamage(uint16_t damage) {
    if (damage >= health) {
        health = 0;
        isAlive = false;
    } else {
        health -= damage;
    }
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

uint16_t Creature::getDamage() const {
    std::cout << "Creature " << name << " attacks with " << damage << " damage!" << std::endl;
    return damage;
}

uint16_t Creature::getMaxHealth() const { return maxHealth; }

uint8_t Creature::getLevel() const { return level; }

uint8_t Creature::getMapId() const {
    return mapId;
}

std::string Creature::getBiomeType() const {
    return biomeType;
}
