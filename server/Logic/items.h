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
    Item();

    void createItem(const std::string& itemName);

    void createItemById(uint8_t itemId);

    bool emptyItem();

    bool longDistance();

    uint8_t getId() const;

    uint16_t getMinDamage() const;

    uint16_t getMaxDamage() const;

    ItemType getType() const;

    uint16_t getHealthRestore() const;

    uint16_t getManaRestore() const;

    uint16_t getManaWaste() const;

    std::string getName() const;

    bool isOffensiveWeapon();
};

#endif
