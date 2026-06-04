#include "items.h"

#include <vector>

#include "toml.hpp"

Item::Item(): id(0) {}

void Item::createItem(const std::string& itemName) {
    name = itemName;
    const toml::value config = toml::parse("server/Logic/items.toml");

    const auto items = toml::find<std::vector<toml::value>>(config, "item");

    for (const auto& item: items) {
        if (toml::find<std::string>(item, "name") == itemName) {
            id = toml::find<uint8_t>(item, "id");
            const auto itemType = toml::find<std::string>(item, "type");

            if (itemType == "WEAPON") {
                type = ItemType::WEAPON;
            } else if (itemType == "MAGIC") {
                type = ItemType::MAGIC;
            } else if (itemType == "HEAL") {
                type = ItemType::HEAL;
            } else if (itemType == "ARMOR") {
                type = ItemType::ARMOR;
            } else if (itemType == "HELMET") {
                type = ItemType::HELMET;
            } else if (itemType == "SHIELD") {
                type = ItemType::SHIELD;
            } else if (itemType == "HEALTH_POTION") {
                type = ItemType::HEALTH_POTION;
            } else if (itemType == "MANA_POTION") {
                type = ItemType::MANA_POTION;
            } else {
                throw std::runtime_error("Unknown item type");
            }

            switch (type) {
                case ItemType::MAGIC:
                    manaCost = toml::find<uint16_t>(item, "manaCost");
                    [[fallthrough]];
                case ItemType::WEAPON:
                    distance = toml::find<bool>(item, "distance");
                    minDamage = toml::find<uint16_t>(item, "minDamage");
                    maxDamage = toml::find<uint16_t>(item, "maxDamage");
                    break;
                case ItemType::HEAL:
                    distance = toml::find<bool>(item, "distance");
                    healthRestore = toml::find<uint16_t>(item, "healthRestore");
                    manaCost = toml::find<uint16_t>(item, "manaCost");
                    break;
                case ItemType::ARMOR:
                case ItemType::HELMET:
                case ItemType::SHIELD:
                    minDefense = toml::find<uint16_t>(item, "defense");
                    maxDefense = toml::find<uint16_t>(item, "defense");
                    break;
                case ItemType::HEALTH_POTION:
                    healthRestore = toml::find<uint16_t>(item, "healthRestore");
                    break;
                case ItemType::MANA_POTION:
                    manaRestore = toml::find<uint16_t>(item, "manaRestore");
                    break;
                default:
                    throw std::runtime_error("Unknown item type");
            }

            break;
        }
    }
}

void Item::createItemById(uint8_t itemId) {
    const toml::value config = toml::parse("server/Logic/items.toml");

    const auto items = toml::find<std::vector<toml::value>>(config, "item");

    for (const auto& item: items) {
        if (toml::find<uint8_t>(item, "id") == itemId) {
            createItem(toml::find<std::string>(item, "name"));
            return;
        }
    }

    throw std::runtime_error("Unknown item id");
}

bool Item::emptyItem() {
    if (id == 0) {
        return true;
    }

    return false;
}

bool Item::longDistance() { return type == ItemType::WEAPON && distance; }

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
