#ifndef MERCHANT_CATALOG_H
#define MERCHANT_CATALOG_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

class MerchantCatalog {
private:
    std::vector<std::pair<uint8_t, uint32_t>> trader;  // (itemId, price)
    std::vector<std::pair<uint8_t, uint32_t>> priest;

    MerchantCatalog();

public:
    using ItemPrice = std::pair<uint8_t, uint32_t>;

    static const MerchantCatalog& instance();

    // Devuelve el catalogo del kind (trader o priest)
    const std::vector<ItemPrice>* getItems(const std::string& kind) const;
};

#endif
