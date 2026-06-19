#include "merchant_catalog.h"

#include <iostream>

#include "../toml.hpp"

MerchantCatalog::MerchantCatalog() {
    const toml::value cfg = toml::parse("server/Logic/config.toml");
    for (const char* kind: {"trader", "priest"}) {
        const auto& arr =
                toml::find<std::vector<toml::value>>(cfg, "merchant", kind, "items");
        auto& vec = (std::string(kind) == "trader") ? trader : priest;
        for (const auto& entry: arr) {
            auto id = toml::find<uint8_t>(entry, "itemId");
            auto price = toml::find<uint32_t>(entry, "price");
            vec.emplace_back(id, price);
        }
    }
}

const MerchantCatalog& MerchantCatalog::instance() {
    static const MerchantCatalog catalog;
    return catalog;
}

const std::vector<MerchantCatalog::ItemPrice>*
MerchantCatalog::getItems(const std::string& kind) const {
    if (kind == "trader") return &trader;
    if (kind == "priest") return &priest;
    return nullptr;
}
