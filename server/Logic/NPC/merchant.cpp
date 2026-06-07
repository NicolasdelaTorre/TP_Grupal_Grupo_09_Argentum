#include "merchant.h"

#include <stdexcept>

Merchant::Merchant(uint16_t id, const std::string& merchant, uint16_t x, uint16_t y) : NPC(id, merchant, x, y) {
    if (merchant == "priest") {
        canRevive = true;
        canHeal = true;
        canSellMagicWeapons = true;
        canSellPotions = true;
        canBuy = false;
        canSellArmory = false;
    } else if (merchant == "trader") {
        canRevive = false;
        canHeal = false;
        canSellMagicWeapons = false;
        canSellPotions = true;
        canBuy = true;
        canSellArmory = true;
    } else {
        throw std::invalid_argument("Invalid merchant type");
    }
}
