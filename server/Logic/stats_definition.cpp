#include "stats_definition.h"

#include <string>

StatsDefinition::StatsDefinition(): attributes("server/Logic/attributes.toml") {}

uint32_t StatsDefinition::maxHealth(uint8_t playerLevel, const std::string& raceName,
                                    const std::string& className) {
    RaceAttribute race = attributes.getRaceAttribute(raceName);
    ClassAttribute class_ = attributes.getClassAttribute(className);
    return race.constitution * class_.FClassHealth * race.FRaceHealth * playerLevel;
}

uint32_t StatsDefinition::recoveryHealth(const std::string& raceName, uint16_t secondsToRecovery) {
    RaceAttribute race = attributes.getRaceAttribute(raceName);
    return race.FRaceRecovery * secondsToRecovery;
}
