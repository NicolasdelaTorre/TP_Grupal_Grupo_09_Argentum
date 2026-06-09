#ifndef STATS_DEFINITION_H
#define STATS_DEFINITION_H

#include <cstdint>

#include "../../common/DTOs.h"

#include "attribute_manager.h"

class StatsDefinition {
private:
    AttributeManager attributes;

public:
    StatsDefinition();

    uint16_t maxHealth(uint8_t playerLevel, RaceCode race, ClassCode class_);

    uint32_t recoveryHealth(RaceCode race);

    uint32_t safeGold(uint8_t playerLevel);

    uint16_t damage(RaceCode race, uint16_t minDamage, uint16_t maxDamage);

    uint16_t maxMana(uint8_t playerLevel, RaceCode race, ClassCode class_);

    uint16_t meditationManaRestore(RaceCode race, ClassCode class_);
};

#endif
