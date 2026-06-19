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

    // El catalog construye los Items leyendo el config.toml y los cachea.
    friend class ItemCatalog;

public:
    Item();

    void createItem(const std::string& itemName);

    void createItemById(uint8_t itemId);

    bool emptyItem() const;

    bool longDistance() const;

    uint8_t getId() const;

    uint16_t getMinDamage() const;

    uint16_t getMaxDamage() const;

    uint16_t getMinDefense() const;

    uint16_t getMaxDefense() const;

    ItemType getType() const;

    uint16_t getHealthRestore() const;

    uint16_t getManaRestore() const;

    uint16_t getManaWaste() const;

    std::string getName() const;

    bool isOffensiveWeapon();
};

#endif
