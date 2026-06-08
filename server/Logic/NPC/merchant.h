#ifndef MERCHANT_H
#define MERCHANT_H

#include <string>
#include <cstdint>

#include "npc.h"

class Merchant : public NPC {
public:
    bool canRevive;
    bool canHeal;
    bool canSellMagicWeapons;
    bool canSellPotions;
    bool canBuy;
    bool canSellArmory;

    Merchant(uint16_t id, const std::string& merchant, uint16_t x, uint16_t y);
};

#endif
