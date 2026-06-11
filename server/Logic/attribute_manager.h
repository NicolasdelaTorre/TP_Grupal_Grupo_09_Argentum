#ifndef ATTRIBUTE_MANAGER_H
#define ATTRIBUTE_MANAGER_H

#include <cstdint>
#include <stdexcept>
#include <string>

#include "../../common/DTOs.h"
#include "toml.hpp"

struct RaceAttribute {
    uint8_t constitution;
    uint8_t force;
    uint8_t intelligence;
    float FRaceHealth;
    float FRaceRecovery;
    float FRaceMana;
};

struct ClassAttribute {
    float FClassHealth;
    float FClassMana;
    float FClassMeditation;
};

// Constantes de las formulas del enunciado.
struct FormulaConstants {
    float goldSafeBase;
    float goldSafeExp;
    float goldMaxFactor;
    float expNextBase;
    float expNextExp;
    uint8_t expKillBonusMaxPct;
    uint8_t expLevelDiffBase;
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

// Rangos de nivel cuando spawnea una criatura segun la zona.
struct CreatureSpawnConfig {
    uint8_t overworldLevelMin;
    uint8_t overworldLevelMax;
    uint8_t dungeonLevelMin;
    uint8_t dungeonLevelMax;
};

struct GameAttributes {
    RaceAttribute human;
    RaceAttribute elf;
    RaceAttribute dwarf;
    RaceAttribute gnome;

    ClassAttribute mage;
    ClassAttribute cleric;
    ClassAttribute champion;
    ClassAttribute warrior;

    FormulaConstants formulas;
    LootConfig loot;
    CreatureSpawnConfig spawn;
};

class AttributeManager {
private:
    GameAttributes attributes;

    RaceAttribute readRace(const toml::value& config, const std::string& raceName);

    ClassAttribute readClass(const toml::value& config, const std::string& className);

public:
    explicit AttributeManager(const std::string& filename);

    RaceAttribute getRaceAttribute(RaceCode race);

    ClassAttribute getClassAttribute(ClassCode class_);

    FormulaConstants getFormulas();

    LootConfig getLootConfig();

    CreatureSpawnConfig getSpawnConfig();
};

#endif
