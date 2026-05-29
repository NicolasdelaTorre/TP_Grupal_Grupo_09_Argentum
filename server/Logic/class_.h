#ifndef CLASS_H
#define CLASS_H

#include <string>
#include <stdexcept>

class Class_ {
    private:
    public:
        enum class ClassCode {
            MAGE = 0,
            CLERIC = 1,
            CHAMPION = 2,
            WARRIOR = 3
        };

        static ClassCode fromString(const std::string& className) {
            if (className == "mage") {
                return ClassCode::MAGE;
            } else if (className == "cleric") {
                return ClassCode::CLERIC;
            } else if (className == "champion") {
                return ClassCode::CHAMPION;
            } else if (className == "warrior") {
                return ClassCode::WARRIOR;
            } else {
                throw std::runtime_error("Unknown class");
            }
        }
};

#endif
