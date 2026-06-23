#ifndef CREATURE_CATALOG_H
#define CREATURE_CATALOG_H

#include <cstdint>
#include <map>
#include <string>

// Stats base de una criatura.
struct CreatureAttributes {
    std::string name;
    uint16_t maxHealth;
    uint16_t damage;
    uint8_t agility;
    uint8_t minArmor;
    uint8_t maxArmor;
    uint8_t minShield;
    uint8_t maxShield;
    uint8_t minHelmet;
    uint8_t maxHelmet;
};

// Rangos de nivel cuando spawnea una criatura segun la zona.
struct SpawnConfig {
    uint8_t overworldLevelMin;
    uint8_t overworldLevelMax;
    uint8_t dungeonLevelMin;
    uint8_t dungeonLevelMax;
};

// Probabilidades y rangos para el drop al matar una criatura.
struct LootConfig {
    uint8_t nothingChance;
    uint8_t goldChance;
    uint8_t potionChance;
    uint8_t itemChance;
    uint8_t goldFactorMinPct;
    uint8_t goldFactorMaxPct;
    uint8_t itemIdMin;
    uint8_t itemIdMax;
    uint8_t potionHealthId;
    uint8_t potionManaId;
};

class CreatureCatalog {
private:
    std::map<std::string, CreatureAttributes> byName;
    SpawnConfig spawn;
    LootConfig loot;

    CreatureCatalog();

public:
    static const CreatureCatalog& instance();

    const CreatureAttributes& findByName(const std::string& name) const;

    const SpawnConfig& getSpawn() const { return spawn; }
    const LootConfig& getLoot() const { return loot; }
};

#endif
