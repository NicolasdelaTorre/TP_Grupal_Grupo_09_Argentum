#include "attribute_manager.h"

#include <iostream>

AttributeManager::AttributeManager(const std::string& filename) {
    const toml::value config = toml::parse(filename);

    attributes.human = readRace(config, "human");
    attributes.elf = readRace(config, "elf");
    attributes.dwarf = readRace(config, "dwarf");
    attributes.gnome = readRace(config, "gnome");

    attributes.mage = readClass(config, "mage");
    attributes.cleric = readClass(config, "cleric");
    attributes.champion = readClass(config, "champion");
    attributes.warrior = readClass(config, "warrior");
}

RaceAttribute AttributeManager::readRace(const toml::value& config, const std::string& raceName) {
    RaceAttribute race;

    race.constitution = toml::find<uint8_t>(config, "race", raceName, "constitution");
    race.FRaceHealth = toml::find<float>(config, "race", raceName, "FRaceHealth");
    race.FRaceRecovery = toml::find<float>(config, "race", raceName, "FRaceRecovery");

    return race;
}

ClassAttribute AttributeManager::readClass(const toml::value& config,
                                           const std::string& className) {
    ClassAttribute clase;

    clase.FClassHealth = toml::find<float>(config, "class", className, "FClassHealth");

    return clase;
}

RaceAttribute AttributeManager::getRaceAttribute(const std::string& raceName) {
    if (raceName == "human") {
        return attributes.human;
    } else if (raceName == "elf") {
        return attributes.elf;
    } else if (raceName == "dwarf") {
        return attributes.dwarf;
    } else if (raceName == "gnome") {
        return attributes.gnome;
    } else {
        throw std::runtime_error("Unknown race");
    }
}

ClassAttribute AttributeManager::getClassAttribute(const std::string& className) {
    if (className == "mage") {
        return attributes.mage;
    } else if (className == "cleric") {
        return attributes.cleric;
    } else if (className == "champion") {
        return attributes.champion;
    } else if (className == "warrior") {
        return attributes.warrior;
    } else {
        throw std::runtime_error("Unknown class");
    }
}
