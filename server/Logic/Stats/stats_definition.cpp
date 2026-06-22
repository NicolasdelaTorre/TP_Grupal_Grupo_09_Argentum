#include "stats_definition.h"

#include <cmath>
#include <cstdlib>

#include "catalog/attribute_catalog.h"
#include "catalog/formula_catalog.h"

uint16_t StatsDefinition::maxHealth(uint8_t playerLevel, RaceCode race, ClassCode class_) const {
    const auto& r = AttributeCatalog::instance().getRace(race);
    const auto& c = AttributeCatalog::instance().getClass(class_);
    return r.constitution * c.FClassHealth * r.FRaceHealth * playerLevel;
}

uint16_t StatsDefinition::maxMana(uint8_t playerLevel, RaceCode race, ClassCode class_) const {
    const auto& r = AttributeCatalog::instance().getRace(race);
    const auto& c = AttributeCatalog::instance().getClass(class_);
    return r.intelligence * c.FClassMana * r.FRaceMana * playerLevel;
}

uint32_t StatsDefinition::safeGold(uint8_t playerLevel) const {
    const auto& f = FormulaCatalog::instance().getFormulas();
    return static_cast<uint32_t>(f.goldSafeBase * std::pow(playerLevel, f.goldSafeExp));
}

uint32_t StatsDefinition::goldMax(uint8_t playerLevel) const {
    const auto& f = FormulaCatalog::instance().getFormulas();
    return static_cast<uint32_t>(safeGold(playerLevel) * f.goldMaxFactor);
}

uint32_t StatsDefinition::nextLevelExp(uint8_t playerLevel) const {
    const auto& f = FormulaCatalog::instance().getFormulas();
    return static_cast<uint32_t>(f.expNextBase * std::pow(playerLevel, f.expNextExp));
}

uint32_t StatsDefinition::recoveryStatThroughTime(RaceCode race) const {
    return AttributeCatalog::instance().getRace(race).FRaceRecovery;
}

uint16_t StatsDefinition::damage(RaceCode race, uint16_t minDamage, uint16_t maxDamage) const {
    const auto& r = AttributeCatalog::instance().getRace(race);
    return r.force * (minDamage + std::rand() % (maxDamage - minDamage + 1));
}

uint16_t StatsDefinition::meditationManaRestore(RaceCode race, ClassCode class_) const {
    const auto& r = AttributeCatalog::instance().getRace(race);
    const auto& c = AttributeCatalog::instance().getClass(class_);
    return c.FClassMeditation * r.intelligence;
}
