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

RaceAttribute AttributeManager::getRaceAttribute(RaceCode race) {
    switch (race) {
        case RaceCode::HUMAN: return attributes.human;
        case RaceCode::ELF:   return attributes.elf;
        case RaceCode::DWARF: return attributes.dwarf;
        case RaceCode::GNOME: return attributes.gnome;
        default: throw std::runtime_error("Unknown race");
    }
}

ClassAttribute AttributeManager::getClassAttribute(ClassCode class_) {
    switch (class_) {
        case ClassCode::MAGE:     return attributes.mage;
        case ClassCode::CLERIC:   return attributes.cleric;
        case ClassCode::CHAMPION: return attributes.champion;
        case ClassCode::WARRIOR:  return attributes.warrior;
        default: throw std::runtime_error("Unknown class");
    }
}
