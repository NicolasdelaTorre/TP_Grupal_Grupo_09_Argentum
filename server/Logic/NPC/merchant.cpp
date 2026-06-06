#include "merchant.h"

Merchant::Merchant(const std::string& merchant) {
    if (merchant == "priest") {
        type = MerchantType::PRIEST;
        canRevive = true;
        canHeal = true;
        canSellMagicWeapons = true;
        canSellPotions = true;
        canBuy = false;
        canSellArmory = false;
    } else if (merchant == "trader") {
        type = MerchantType::TRADER;
        canRevive = false;
        canHeal = false;
        canSellMagicWeapons = false;
        canSellPotions = true;
        canBuy = true;
        canSellArmory = true;
    }
}
