#ifndef CLASS_H
#define CLASS_H

#include <stdexcept>
#include <string>

#include "../../common/DTOs.h"

// Helpers de parseo/log para ClassCode. El gameplay maneja ClassCode directo;
// estos helpers solo se usan para leer secciones del TOML de atributos y para
// imprimir en logs.
class PlayerClass {
public:
    static std::string toString(ClassCode code) {
        switch (code) {
            case ClassCode::MAGE: return "Mage";
            case ClassCode::CLERIC: return "Cleric";
            case ClassCode::CHAMPION: return "Champion";
            case ClassCode::WARRIOR: return "Warrior";
            default: throw std::runtime_error("Unknown class");
        }
    }
};

#endif
