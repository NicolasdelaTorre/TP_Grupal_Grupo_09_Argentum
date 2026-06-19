#include "items.h"

#include <stdexcept>

#include "catalog/item_catalog.h"

Item::Item(): id(0) {}

void Item::createItem(const std::string& itemName) {
    *this = ItemCatalog::instance().findByName(itemName);
}

void Item::createItemById(uint8_t itemId) {
    *this = ItemCatalog::instance().findById(itemId);
}

bool Item::emptyItem() const {
    if (id == 0) {
        return true;
    }

    return false;
}

bool Item::longDistance() const {
    return (type == ItemType::WEAPON && distance) || type == ItemType::MAGIC;
}

uint8_t Item::getId() const {
    if (id == 0) {
        throw std::runtime_error("Item Error: trying to get id of empty item");
    }
    return id;
}

uint16_t Item::getMinDamage() const {
    if (type != ItemType::WEAPON && type != ItemType::MAGIC) {
        throw std::runtime_error("Item Error: trying to get damage of non-weapon item");
    }
    return minDamage;
}

uint16_t Item::getMaxDamage() const {
    if (type != ItemType::WEAPON && type != ItemType::MAGIC) {
        throw std::runtime_error("Item Error: trying to get damage of non-weapon item");
    }
    return maxDamage;
}

uint16_t Item::getMinDefense() const {
    if (type != ItemType::ARMOR && type != ItemType::HELMET && type != ItemType::SHIELD) {
        throw std::runtime_error("Item Error: trying to get defense of non-defensive item");
    }
    return minDefense;
}

uint16_t Item::getMaxDefense() const {
    if (type != ItemType::ARMOR && type != ItemType::HELMET && type != ItemType::SHIELD) {
        throw std::runtime_error("Item Error: trying to get defense of non-defensive item");
    }
    return maxDefense;
}

ItemType Item::getType() const {
    if (id == 0) {
        throw std::runtime_error("Item Error: trying to get type of empty item");
    }
    return type;
}

uint16_t Item::getHealthRestore() const {
    if (type == ItemType::HEAL || type == ItemType::HEALTH_POTION) {
        return healthRestore;
    }

    return 0;
}

uint16_t Item::getManaRestore() const {
    if (type == ItemType::MANA_POTION) {
        return manaRestore;
    }

    return 0;
}

uint16_t Item::getManaWaste() const {
    if (type == ItemType::MAGIC || type == ItemType::HEAL) {
        return manaCost;
    }

    return 0;
}

std::string Item::getName() const {
    if (id == 0) {
        throw std::runtime_error("Item Error: trying to get name of empty item");
    }
    return name;
}

bool Item::isOffensiveWeapon() { return type == ItemType::WEAPON || type == ItemType::MAGIC; }
