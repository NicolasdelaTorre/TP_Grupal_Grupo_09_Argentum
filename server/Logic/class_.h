#ifndef CLASS_H
#define CLASS_H

#include <stdexcept>
#include <string>

class Class_ {
private:
public:
    enum class ClassCode : uint8_t { MAGE = 0, CLERIC = 1, CHAMPION = 2, WARRIOR = 3 };

    static ClassCode fromString(const std::string& className) {
        if (className == "Mage") {
            return ClassCode::MAGE;
        } else if (className == "Cleric") {
            return ClassCode::CLERIC;
        } else if (className == "Champion") {
            return ClassCode::CHAMPION;
        } else if (className == "Warrior") {
            return ClassCode::WARRIOR;
        } else {
            throw std::runtime_error("Unknown class");
        }
    }
};

#endif
