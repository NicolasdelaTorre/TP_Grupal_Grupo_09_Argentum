#ifndef STATS_DEFINITION_H
#define STATS_DEFINITION_H

#include <cstdint>
#include <string>

#include "attribute_manager.h"

class StatsDefinition {
private:
    AttributeManager attributes;

public:
    StatsDefinition();

    uint16_t maxHealth(uint8_t playerLevel, const std::string& raceName,
                       const std::string& className);

    uint32_t recoveryHealth(const std::string& raceName, uint16_t secondsToRecovery);

    uint32_t safeGold(uint8_t playerLevel);

    uint16_t damage(const std::string& raceName, uint16_t minDamage, uint16_t maxDamage);
};

#endif
