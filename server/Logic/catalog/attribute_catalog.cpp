#include "attribute_catalog.h"

#include <stdexcept>
#include <string>

#include "../toml.hpp"

static RaceAttributes readRace(const toml::value& cfg, const std::string& raceName) {
    RaceAttributes r;
    r.constitution = toml::find<uint8_t>(cfg, "race", raceName, "constitution");
    r.force = toml::find<uint8_t>(cfg, "race", raceName, "force");
    r.intelligence = toml::find<uint8_t>(cfg, "race", raceName, "intelligence");
    r.agility = toml::find<uint8_t>(cfg, "race", raceName, "agility");
    r.FRaceHealth = toml::find<float>(cfg, "race", raceName, "FRaceHealth");
    r.FRaceRecovery = toml::find<float>(cfg, "race", raceName, "FRaceRecovery");
    r.FRaceMana = toml::find<float>(cfg, "race", raceName, "FRaceMana");
    return r;
}

static ClassAttributes readClass(const toml::value& cfg, const std::string& className) {
    ClassAttributes c;
    c.FClassHealth = toml::find<float>(cfg, "class", className, "FClassHealth");
    c.FClassMana = toml::find<float>(cfg, "class", className, "FClassMana");
    c.FClassMeditation = toml::find<float>(cfg, "class", className, "FClassMeditation");
    return c;
}

AttributeCatalog::AttributeCatalog() {
    const toml::value cfg = toml::parse("server/Logic/config.toml");

    human = readRace(cfg, "human");
    elf = readRace(cfg, "elf");
    dwarf = readRace(cfg, "dwarf");
    gnome = readRace(cfg, "gnome");

    mage = readClass(cfg, "mage");
    cleric = readClass(cfg, "cleric");
    champion = readClass(cfg, "champion");
    warrior = readClass(cfg, "warrior");
}

const AttributeCatalog& AttributeCatalog::instance() {
    static const AttributeCatalog catalog;
    return catalog;
}

const RaceAttributes& AttributeCatalog::getRace(RaceCode race) const {
    switch (race) {
        case RaceCode::HUMAN: return human;
        case RaceCode::ELF:   return elf;
        case RaceCode::DWARF: return dwarf;
        case RaceCode::GNOME: return gnome;
        default: throw std::runtime_error("AttributeCatalog: race desconocida");
    }
}

const ClassAttributes& AttributeCatalog::getClass(ClassCode class_) const {
    switch (class_) {
        case ClassCode::MAGE:     return mage;
        case ClassCode::CLERIC:   return cleric;
        case ClassCode::CHAMPION: return champion;
        case ClassCode::WARRIOR:  return warrior;
        default: throw std::runtime_error("AttributeCatalog: class desconocida");
    }
}
