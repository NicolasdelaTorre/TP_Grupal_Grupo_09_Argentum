#include "creature.h"

#include <ctime>

#include "../toml.hpp"

Creature::Creature(uint16_t id, const std::string& name, uint8_t mapId, uint16_t x, uint16_t y) : NPC(id, name, x, y) {
    // Set level
    srand(time(nullptr));
    if (mapId == 0) {
        // Overworld
        level = 1 + rand() % 5;
    } else {
        // Dungeons
        level = 5 + rand() % 10;
    }

    const toml::value config = toml::parse("server/Logic/NPC/npc.toml");

    const auto npcs = toml::find<std::vector<toml::value>>(config, "npc");

    for (const auto& npc: npcs) {
        if (toml::find<std::string>(npc, "name") == name) {
            maxHealth = toml::find<uint16_t>(npc, "health") * level;
            health = maxHealth;
            damage = toml::find<uint16_t>(npc, "damage") * level;
            break;
        }
    }
}

void Creature::stalkPlayer(Position playerPosition) {
    // Position playerPosition = map.searchPlayer(position.x, position.y);

    // No player in sight
    if (playerPosition.x == -1)
        return;

    int16_t dx = playerPosition.x - position.x;
    int16_t dy = playerPosition.y - position.y;

    position.x += (dx > 0) - (dx < 0);
    position.y += (dy > 0) - (dy < 0);
}

uint16_t Creature::attackPlayer(uint8_t playerId) {
    // uint8_t playerId = map.nextEntity(position.x, position.y, false);

    if (!playerId) {
        return 0;
    }

    return damage;
}

void Creature::receiveDamage(uint16_t damage) {
    if (damage >= health) {
        health = 0;
    } else {
        health -= damage;
    }
}
