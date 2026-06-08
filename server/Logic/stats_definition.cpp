#include "stats_definition.h"

#include <cstdlib>
#include <ctime>
#include <string>

StatsDefinition::StatsDefinition(): attributes("server/Logic/attributes.toml") {}

uint16_t StatsDefinition::maxHealth(uint8_t playerLevel, const std::string& raceName,
                                    const std::string& className) {
    RaceAttribute race = attributes.getRaceAttribute(raceName);
    ClassAttribute class_ = attributes.getClassAttribute(className);
    return race.constitution * class_.FClassHealth * race.FRaceHealth * playerLevel;
}

uint32_t StatsDefinition::recoveryHealth(const std::string& raceName) {
    RaceAttribute race = attributes.getRaceAttribute(raceName);
    return race.FRaceRecovery;
}

uint32_t StatsDefinition::safeGold(uint8_t playerLevel) { return 100 * std::pow(playerLevel, 1.1); }

uint16_t StatsDefinition::damage(const std::string& raceName, uint16_t minDamage,
                                 uint16_t maxDamage) {
    RaceAttribute race = attributes.getRaceAttribute(raceName);

    srand(time(nullptr));
    return race.force * (minDamage + rand() % (maxDamage - minDamage + 1));
}

uint16_t StatsDefinition::maxMana(uint8_t playerLevel, const std::string& raceName,
                                  const std::string& className) {
    RaceAttribute race = attributes.getRaceAttribute(raceName);
    ClassAttribute class_ = attributes.getClassAttribute(className);
    return race.intelligence * class_.FClassMana * race.FRaceMana * playerLevel;
}

uint16_t StatsDefinition::meditationManaRestore(const std::string& raceName, const std::string& className) {
    RaceAttribute race = attributes.getRaceAttribute(raceName);
    ClassAttribute class_ = attributes.getClassAttribute(className);
    return class_.FClassMeditation * race.intelligence;
}
