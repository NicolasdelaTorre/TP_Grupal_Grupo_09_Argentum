#ifndef ITEMS_H
#define ITEMS_H

#include <cstdint>
#include <string>

enum class ItemType : uint8_t {
    WEAPON,
    MAGIC,
    HEAL,
    ARMOR,
    HELMET,
    SHIELD,
    HEALTH_POTION,
    MANA_POTION
};

class Item {
private:
    uint8_t id;
    std::string name;
    ItemType type;

    bool distance;

    uint16_t minDamage;
    uint16_t maxDamage;

    uint16_t minDefense;
    uint16_t maxDefense;

    uint16_t manaCost;

    uint16_t healthRestore;
    uint16_t manaRestore;

public:
    explicit Item(const std::string& itemName);
};

#endif
