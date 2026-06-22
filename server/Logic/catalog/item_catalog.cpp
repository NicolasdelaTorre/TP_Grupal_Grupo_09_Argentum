#include "item_catalog.h"

#include <stdexcept>
#include <vector>

#include "../toml.hpp"

// Helper local del constructor: traduce el string del item al enum ItemType.
static ItemType parseItemType(const std::string& typeStr) {
    if (typeStr == "WEAPON") return ItemType::WEAPON;
    if (typeStr == "MAGIC") return ItemType::MAGIC;
    if (typeStr == "HEAL") return ItemType::HEAL;
    if (typeStr == "ARMOR") return ItemType::ARMOR;
    if (typeStr == "HELMET") return ItemType::HELMET;
    if (typeStr == "SHIELD") return ItemType::SHIELD;
    if (typeStr == "HEALTH_POTION") return ItemType::HEALTH_POTION;
    if (typeStr == "MANA_POTION") return ItemType::MANA_POTION;
    throw std::runtime_error("Item Catalog: tipo de item desconocido: " + typeStr);
}

ItemCatalog::ItemCatalog() {
    const toml::value config = toml::parse("server/Logic/Stats/config.toml");
    const auto items = toml::find<std::vector<toml::value>>(config, "item");

    for (const auto& entry: items) {
        Item item;
        item.id = toml::find<uint8_t>(entry, "id");
        item.name = toml::find<std::string>(entry, "name");
        item.type = parseItemType(toml::find<std::string>(entry, "type"));

        switch (item.type) {
            case ItemType::MAGIC:
                item.manaCost = toml::find<uint16_t>(entry, "manaCost");
                item.distance = toml::find<bool>(entry, "distance");
                item.minDamage = toml::find<uint16_t>(entry, "minDamage");
                item.maxDamage = toml::find<uint16_t>(entry, "maxDamage");
                break;
            case ItemType::WEAPON:
                item.distance = toml::find<bool>(entry, "distance");
                item.minDamage = toml::find<uint16_t>(entry, "minDamage");
                item.maxDamage = toml::find<uint16_t>(entry, "maxDamage");
                break;
            case ItemType::HEAL:
                item.distance = toml::find<bool>(entry, "distance");
                item.healthRestore = toml::find<uint16_t>(entry, "healthRestore");
                item.manaCost = toml::find<uint16_t>(entry, "manaCost");
                break;
            case ItemType::ARMOR:
            case ItemType::HELMET:
            case ItemType::SHIELD:
                item.minDefense = toml::find<uint16_t>(entry, "minDefense");
                item.maxDefense = toml::find<uint16_t>(entry, "maxDefense");
                break;
            case ItemType::HEALTH_POTION:
                item.healthRestore = toml::find<uint16_t>(entry, "healthRestore");
                break;
            case ItemType::MANA_POTION:
                item.manaRestore = toml::find<uint16_t>(entry, "manaRestore");
                break;
        }

        nameToId[item.name] = item.id;
        byId[item.id] = item;
    }
}

const ItemCatalog& ItemCatalog::instance() {
    static const ItemCatalog catalog;
    return catalog;
}

const Item& ItemCatalog::findByName(const std::string& name) const {
    auto it = nameToId.find(name);
    if (it == nameToId.end()) {
        throw std::runtime_error("Item Catalog: nombre desconocido: " + name);
    }
    return byId.at(it->second);
}

const Item& ItemCatalog::findById(uint8_t id) const {
    auto it = byId.find(id);
    if (it == byId.end()) {
        throw std::runtime_error("Item Catalog: id desconocido: " + std::to_string(id));
    }
    return it->second;
}
