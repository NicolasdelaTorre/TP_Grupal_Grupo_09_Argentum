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
    return 100 * std::pow(playerLevel, 1.1);
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
