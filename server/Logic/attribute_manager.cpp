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
    race.force = toml::find<uint8_t>(config, "race", raceName, "force");
    race.intelligence = toml::find<uint8_t>(config, "race", raceName, "intelligence");
    race.FRaceHealth = toml::find<float>(config, "race", raceName, "FRaceHealth");
    race.FRaceRecovery = toml::find<float>(config, "race", raceName, "FRaceRecovery");
    race.FRaceMana = toml::find<float>(config, "race", raceName, "FRaceMana");

    return race;
}

ClassAttribute AttributeManager::readClass(const toml::value& config,
                                           const std::string& className) {
    ClassAttribute clase;

    clase.FClassHealth = toml::find<float>(config, "class", className, "FClassHealth");
    clase.FClassMana = toml::find<float>(config, "class", className, "FClassMana");
    clase.FClassMeditation = toml::find<float>(config, "class", className, "FClassMeditation");

    return clase;
}

RaceAttribute AttributeManager::getRaceAttribute(const std::string& raceName) {
    if (raceName == "Human") {
        return attributes.human;
    } else if (raceName == "Elf") {
        return attributes.elf;
    } else if (raceName == "Dwarf") {
        return attributes.dwarf;
    } else if (raceName == "Gnome") {
        return attributes.gnome;
    } else {
        throw std::runtime_error("Unknown race");
    }
}

ClassAttribute AttributeManager::getClassAttribute(const std::string& className) {
    if (className == "Mage") {
        return attributes.mage;
    } else if (className == "Cleric") {
        return attributes.cleric;
    } else if (className == "Champion") {
        return attributes.champion;
    } else if (className == "Warrior") {
        return attributes.warrior;
    } else {
        throw std::runtime_error("Unknown class");
    }
}
