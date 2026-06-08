#ifndef MERCHANT_H
#define MERCHANT_H

#include <string>
#include <cstdint>

enum class MerchantType : uint8_t { PRIEST = 0, TRADER };

class Merchant {
private:
    MerchantType type;

public:
    bool canRevive;
    bool canHeal;
    bool canSellMagicWeapons;
    bool canSellPotions;
    bool canBuy;
    bool canSellArmory;

    Merchant(const std::string& merchant);
};

#endif
