#include "creature_catalog.h"

#include <stdexcept>
#include <vector>

#include "../toml.hpp"

CreatureCatalog::CreatureCatalog() {
    const toml::value cfg = toml::parse("server/Logic/config.toml");

    const auto npcs = toml::find<std::vector<toml::value>>(cfg, "npc");
    for (const auto& npc: npcs) {
        CreatureAttributes c;
        c.name = toml::find<std::string>(npc, "name");
        c.maxHealth = toml::find<uint16_t>(npc, "maxHealth");
        c.damage = toml::find<uint16_t>(npc, "damage");
        c.agility = toml::find<uint8_t>(npc, "agility");
        c.minArmor = toml::find<uint8_t>(npc, "minArmor");
        c.maxArmor = toml::find<uint8_t>(npc, "maxArmor");
        c.minShield = toml::find<uint8_t>(npc, "minShield");
        c.maxShield = toml::find<uint8_t>(npc, "maxShield");
        c.minHelmet = toml::find<uint8_t>(npc, "minHelmet");
        c.maxHelmet = toml::find<uint8_t>(npc, "maxHelmet");
        byName[c.name] = c;
    }

    spawn.overworldLevelMin = toml::find<uint8_t>(cfg, "creature", "spawn", "overworldLevelMin");
    spawn.overworldLevelMax = toml::find<uint8_t>(cfg, "creature", "spawn", "overworldLevelMax");
    spawn.dungeonLevelMin = toml::find<uint8_t>(cfg, "creature", "spawn", "dungeonLevelMin");
    spawn.dungeonLevelMax = toml::find<uint8_t>(cfg, "creature", "spawn", "dungeonLevelMax");

    loot.nothingChance = toml::find<uint8_t>(cfg, "loot", "npc", "nothingChance");
    loot.goldChance = toml::find<uint8_t>(cfg, "loot", "npc", "goldChance");
    loot.potionChance = toml::find<uint8_t>(cfg, "loot", "npc", "potionChance");
    loot.itemChance = toml::find<uint8_t>(cfg, "loot", "npc", "itemChance");
    loot.goldFactorMinPct = toml::find<uint8_t>(cfg, "loot", "npc", "goldFactorMinPct");
    loot.goldFactorMaxPct = toml::find<uint8_t>(cfg, "loot", "npc", "goldFactorMaxPct");
    loot.itemIdMin = toml::find<uint8_t>(cfg, "loot", "npc", "itemIdMin");
    loot.itemIdMax = toml::find<uint8_t>(cfg, "loot", "npc", "itemIdMax");
    loot.potionHealthId = toml::find<uint8_t>(cfg, "loot", "npc", "potionHealthId");
    loot.potionManaId = toml::find<uint8_t>(cfg, "loot", "npc", "potionManaId");
}

const CreatureCatalog& CreatureCatalog::instance() {
    static const CreatureCatalog catalog;
    return catalog;
}

const CreatureAttributes& CreatureCatalog::findByName(const std::string& name) const {
    auto it = byName.find(name);
    if (it == byName.end()) {
        throw std::runtime_error("Creature Catalog: tipo desconocido: " + name);
    }
    return it->second;
}
