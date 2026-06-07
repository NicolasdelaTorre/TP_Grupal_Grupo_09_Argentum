#ifndef RACE_H
#define RACE_H

#include <cstdint>
#include <stdexcept>
#include <string>

#include "../../common/DTOs.h"

class Race {
private:
public:
    static RaceCode fromString(const std::string& raceName) {
        if (raceName == "Human") {
            return RaceCode::HUMAN;
        } else if (raceName == "Elf") {
            return RaceCode::ELF;
        } else if (raceName == "Dwarf") {
            return RaceCode::DWARF;
        } else if (raceName == "Gnome") {
            return RaceCode::GNOME;
        } else {
            throw std::runtime_error("Unknown race");
        }
    }

    static const char* toString(RaceCode code) {
        switch (code) {
            case RaceCode::HUMAN:
                return "Human";
            case RaceCode::ELF:
                return "Elf";
            case RaceCode::DWARF:
                return "Dwarf";
            case RaceCode::GNOME:
                return "Gnome";
            default:
                throw std::runtime_error("Invalid RaceCode");
        }
    }
};

#endif
