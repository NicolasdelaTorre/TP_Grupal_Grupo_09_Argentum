#include "creature.h"

#include <ctime>

#include "../toml.hpp"
#include <iostream>

Creature::Creature(uint16_t id, const std::string& name, uint8_t mapId, uint16_t x, uint16_t y) : NPC(id, name, x, y), isAlive(true) {
    // Set level
    srand(time(nullptr));
    if (mapId == 0) {
        // Overworld
        level = 1 + rand() % 5;
    } else {
        // Dungeons
        level = 5 + rand() % 6;
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
              << " and damage " << damage << "in position (" << position.x << ", " << position.y << ")" << std::endl;
}

void Creature::stalkPlayer(Position playerPosition) {
    // No player in sight
    if (playerPosition.x == -1)
        return;

    int16_t dx = playerPosition.x - position.x;
    int16_t dy = playerPosition.y - position.y;

    position.x += (dx > 0) - (dx < 0);
    position.y += (dy > 0) - (dy < 0);
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
