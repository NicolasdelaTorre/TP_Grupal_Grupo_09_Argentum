#ifndef STATS_DEFINITION_H
#define STATS_DEFINITION_H

#include <cstdint>

#include "../../common/DTOs.h"

class StatsDefinition {
public:
    uint16_t maxHealth(uint8_t playerLevel, RaceCode race, ClassCode class_) const;
    uint16_t maxMana(uint8_t playerLevel, RaceCode race, ClassCode class_) const;

    uint32_t safeGold(uint8_t playerLevel) const;
    uint32_t goldMax(uint8_t playerLevel) const;
    uint32_t nextLevelExp(uint8_t playerLevel) const;

    uint32_t recoveryStatThroughTime(RaceCode race) const;

    uint16_t damage(RaceCode race, uint16_t minDamage, uint16_t maxDamage) const;

    uint16_t meditationManaRestore(RaceCode race, ClassCode class_) const;
};

#endif
