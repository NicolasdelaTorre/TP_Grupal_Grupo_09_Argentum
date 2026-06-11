#include "attribute_manager.h"

#include <iostream>

AttributeManager::AttributeManager(const std::string& filename) {
    const toml::value config = toml::parse(filename);

    attributes.human = readRace(config, "human");
    attributes.elf = readRace(config, "elf");
    attributes.dwarf = readRace(config, "dwarf");
    attributes.gnome = readRace(config, "gnome");

    attributes.mage = readClass(config, "mage");
    attributes.cleric = readClass(config, "cleric");
    attributes.champion = readClass(config, "champion");
    attributes.warrior = readClass(config, "warrior");

    attributes.formulas.goldSafeBase = toml::find<float>(config, "formula", "gold", "safeBase");
    attributes.formulas.goldSafeExp = toml::find<float>(config, "formula", "gold", "safeExp");
    attributes.formulas.goldMaxFactor = toml::find<float>(config, "formula", "gold", "maxFactor");
    attributes.formulas.expNextBase = toml::find<float>(config, "formula", "experience", "nextBase");
    attributes.formulas.expNextExp = toml::find<float>(config, "formula", "experience", "nextExp");
    attributes.formulas.expKillBonusMaxPct = toml::find<uint8_t>(config, "formula", "experience", "killBonusMaxPct");
    attributes.formulas.expLevelDiffBase = toml::find<uint8_t>(config, "formula", "experience", "levelDiffBase");
    attributes.formulas.evadeThreshold = toml::find<float>(config, "formula", "combat", "evadeThreshold");
    attributes.formulas.criticalChancePct = toml::find<uint8_t>(config, "formula", "combat", "criticalChancePct");
    attributes.formulas.fairPlayNewbieLevel = toml::find<uint8_t>(config, "formula", "fairplay", "newbieLevel");
    attributes.formulas.fairPlayMaxLevelDiff = toml::find<uint8_t>(config, "formula", "fairplay", "maxLevelDiff");

    attributes.loot.nothingChance = toml::find<uint8_t>(config, "loot", "npc", "nothingChance");
    attributes.loot.goldChance = toml::find<uint8_t>(config, "loot", "npc", "goldChance");
    attributes.loot.potionChance = toml::find<uint8_t>(config, "loot", "npc", "potionChance");
    attributes.loot.itemChance = toml::find<uint8_t>(config, "loot", "npc", "itemChance");
    attributes.loot.goldFactorMinPct = toml::find<uint8_t>(config, "loot", "npc", "goldFactorMinPct");
    attributes.loot.goldFactorMaxPct = toml::find<uint8_t>(config, "loot", "npc", "goldFactorMaxPct");
    attributes.loot.itemIdMin = toml::find<uint8_t>(config, "loot", "npc", "itemIdMin");
    attributes.loot.itemIdMax = toml::find<uint8_t>(config, "loot", "npc", "itemIdMax");
    attributes.loot.potionHealthId = toml::find<uint8_t>(config, "loot", "npc", "potionHealthId");
    attributes.loot.potionManaId = toml::find<uint8_t>(config, "loot", "npc", "potionManaId");

    attributes.spawn.overworldLevelMin = toml::find<uint8_t>(config, "creature", "spawn", "overworldLevelMin");
    attributes.spawn.overworldLevelMax = toml::find<uint8_t>(config, "creature", "spawn", "overworldLevelMax");
    attributes.spawn.dungeonLevelMin = toml::find<uint8_t>(config, "creature", "spawn", "dungeonLevelMin");
    attributes.spawn.dungeonLevelMax = toml::find<uint8_t>(config, "creature", "spawn", "dungeonLevelMax");
}

RaceAttribute AttributeManager::readRace(const toml::value& config, const std::string& raceName) {
    RaceAttribute race;

    race.constitution = toml::find<uint8_t>(config, "race", raceName, "constitution");
    race.force = toml::find<uint8_t>(config, "race", raceName, "force");
    race.intelligence = toml::find<uint8_t>(config, "race", raceName, "intelligence");
    race.agility = toml::find<uint8_t>(config, "race", raceName, "agility");
    race.FRaceHealth = toml::find<float>(config, "race", raceName, "FRaceHealth");
    race.FRaceRecovery = toml::find<float>(config, "race", raceName, "FRaceRecovery");
    race.FRaceMana = toml::find<float>(config, "race", raceName, "FRaceMana");

    return race;
}

ClassAttribute AttributeManager::readClass(const toml::value& config,
                                           const std::string& className) {
    ClassAttribute clase;

    clase.FClassHealth = toml::find<float>(config, "class", className, "FClassHealth");
    clase.FClassMana = toml::find<float>(config, "class", className, "FClassMana");
    clase.FClassMeditation = toml::find<float>(config, "class", className, "FClassMeditation");

    return clase;
}

RaceAttribute AttributeManager::getRaceAttribute(RaceCode race) {
    switch (race) {
        case RaceCode::HUMAN: return attributes.human;
        case RaceCode::ELF:   return attributes.elf;
        case RaceCode::DWARF: return attributes.dwarf;
        case RaceCode::GNOME: return attributes.gnome;
        default: throw std::runtime_error("Unknown race");
    }
}

ClassAttribute AttributeManager::getClassAttribute(ClassCode class_) {
    switch (class_) {
        case ClassCode::MAGE:     return attributes.mage;
        case ClassCode::CLERIC:   return attributes.cleric;
        case ClassCode::CHAMPION: return attributes.champion;
        case ClassCode::WARRIOR:  return attributes.warrior;
        default: throw std::runtime_error("Unknown class");
    }
}

FormulaConstants AttributeManager::getFormulas() { return attributes.formulas; }

LootConfig AttributeManager::getLootConfig() { return attributes.loot; }

CreatureSpawnConfig AttributeManager::getSpawnConfig() { return attributes.spawn; }
