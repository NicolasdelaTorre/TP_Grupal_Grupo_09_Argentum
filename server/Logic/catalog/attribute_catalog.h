#ifndef ATTRIBUTE_CATALOG_H
#define ATTRIBUTE_CATALOG_H

#include <cstdint>

#include "../../../common/DTOs.h"

// Atributos base de una raza.
struct RaceAttributes {
    uint8_t constitution;
    uint8_t force;
    uint8_t intelligence;
    uint8_t agility;
    float FRaceHealth;
    float FRaceRecovery;
    float FRaceMana;
};

// Multiplicadores por clase.
struct ClassAttributes {
    float FClassHealth;
    float FClassMana;
    float FClassMeditation;
};

class AttributeCatalog {
private:
    RaceAttributes human, elf, dwarf, gnome;
    ClassAttributes mage, cleric, champion, warrior;

    AttributeCatalog();

public:
    static const AttributeCatalog& instance();

    const RaceAttributes& getRace(RaceCode race) const;
    const ClassAttributes& getClass(ClassCode class_) const;
};

#endif
