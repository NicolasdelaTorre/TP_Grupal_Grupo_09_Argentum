#include "items.h"

#include <vector>

#include "toml.hpp"

Item::Item(const std::string& itemName): name(itemName) {
    name = itemName;
    const toml::value config = toml::parse("server/Logic/items.toml");

    const auto& items = toml::find<std::vector<toml::value>>(config, "items");

    for (const auto& item: items) {
        if (toml::find<std::string>(item, "name") == itemName) {
            id = toml::find<uint8_t>(item, "id");
            type = static_cast<ItemType>(toml::find<uint8_t>(item, "type"));

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
