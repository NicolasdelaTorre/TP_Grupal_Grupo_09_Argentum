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

struct GameAttributes {
    RaceAttribute human;
    RaceAttribute elf;
    RaceAttribute dwarf;
    RaceAttribute gnome;

    ClassAttribute mage;
    ClassAttribute cleric;
    ClassAttribute champion;
    ClassAttribute warrior;
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
};

#endif
