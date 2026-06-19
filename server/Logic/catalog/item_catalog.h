#ifndef ITEM_CATALOG_H
#define ITEM_CATALOG_H

#include <cstdint>
#include <map>
#include <string>

#include "../items.h"

class ItemCatalog {
private:
    std::map<uint8_t, Item> byId;
    std::map<std::string, uint8_t> nameToId;

    ItemCatalog();

public:
    static const ItemCatalog& instance();

    const Item& findByName(const std::string& name) const;
    const Item& findById(uint8_t id) const;
};

#endif
