#ifndef RACE_H
#define RACE_H

#include <stdexcept>
#include <string>

#include "../../common/DTOs.h"

// Helper de log para RaceCode. El gameplay maneja RaceCode directo.
class Race {
public:
    static const char* toString(RaceCode code) {
        switch (code) {
            case RaceCode::HUMAN: return "Human";
            case RaceCode::ELF: return "Elf";
            case RaceCode::DWARF: return "Dwarf";
            case RaceCode::GNOME: return "Gnome";
            default: throw std::runtime_error("Invalid RaceCode");
        }
    }
};

#endif
