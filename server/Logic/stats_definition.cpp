#include "stats_definition.h"

#include <cmath>
#include <cstdlib>
#include <ctime>

StatsDefinition::StatsDefinition(): attributes("server/Logic/attributes.toml") {}

uint16_t StatsDefinition::maxHealth(uint8_t playerLevel, RaceCode race, ClassCode class_) {
    RaceAttribute r = attributes.getRaceAttribute(race);
    ClassAttribute c = attributes.getClassAttribute(class_);
    return r.constitution * c.FClassHealth * r.FRaceHealth * playerLevel;
}

uint32_t StatsDefinition::recoveryStatThroughTime(RaceCode race) {
    return attributes.getRaceAttribute(race).FRaceRecovery;
}

uint32_t StatsDefinition::safeGold(uint8_t playerLevel) {
    FormulaConstants f = attributes.getFormulas();
    return static_cast<uint32_t>(f.goldSafeBase * std::pow(playerLevel, f.goldSafeExp));
}

uint32_t StatsDefinition::goldMax(uint8_t playerLevel) {
    // safeGold * factor: el jugador puede llevar un excedente sobre el umbral seguro.
    FormulaConstants f = attributes.getFormulas();
    return static_cast<uint32_t>(safeGold(playerLevel) * f.goldMaxFactor);
}

uint32_t StatsDefinition::nextLevelExp(uint8_t playerLevel) {
    FormulaConstants f = attributes.getFormulas();
    return static_cast<uint32_t>(f.expNextBase * std::pow(playerLevel, f.expNextExp));
}

uint16_t StatsDefinition::damage(RaceCode race, uint16_t minDamage, uint16_t maxDamage) {
    RaceAttribute r = attributes.getRaceAttribute(race);
    srand(time(nullptr));
    return r.force * (minDamage + rand() % (maxDamage - minDamage + 1));
}

uint16_t StatsDefinition::maxMana(uint8_t playerLevel, RaceCode race, ClassCode class_) {
    RaceAttribute r = attributes.getRaceAttribute(race);
    ClassAttribute c = attributes.getClassAttribute(class_);
    return r.intelligence * c.FClassMana * r.FRaceMana * playerLevel;
}

uint16_t StatsDefinition::meditationManaRestore(RaceCode race, ClassCode class_) {
    RaceAttribute r = attributes.getRaceAttribute(race);
    ClassAttribute c = attributes.getClassAttribute(class_);
    return c.FClassMeditation * r.intelligence;
}

FormulaConstants StatsDefinition::getFormulas() { return attributes.getFormulas(); }

LootConfig StatsDefinition::getLootConfig() { return attributes.getLootConfig(); }

CreatureSpawnConfig StatsDefinition::getSpawnConfig() { return attributes.getSpawnConfig(); }
