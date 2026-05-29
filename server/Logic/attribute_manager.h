#ifndef ATTRIBUTE_MANAGER_H
#define ATTRIBUTE_MANAGER_H

#include <cstdint>
#include <stdexcept>
#include <string>

#include "toml.hpp"

struct RaceAttribute {
    uint8_t constitution;
    float FRaceHealth;
    float FRaceRecovery;
    uint8_t SecondsToRecovery;
};

struct ClassAttribute {
    float FClassHealth;
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

    RaceAttribute getRaceAttribute(const std::string& raceName);

    ClassAttribute getClassAttribute(const std::string& className);
};

#endif
