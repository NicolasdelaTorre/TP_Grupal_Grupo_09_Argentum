#include "banker.h"

Banker::Banker(uint16_t id, const std::string& name, uint16_t x, uint16_t y) : NPC(id, name, x, y, 0), parser() {}

void Banker::addPlayer(const std::string& name) {
    for (const auto& account : accounts) {
        if (account.name == name) return;
    }

    if (parser.checkBankAccountExists(name)) {
        BankAccount account = parser.loadBankAccount(name);
        accounts.push_back(std::move(account));
    } else {
        BankAccount newAccount{name, 0, {}};
        parser.saveBankAccount(newAccount);
        accounts.push_back(std::move(newAccount));
    }
}

bool Banker::depositGold(const std::string& name, uint32_t amount) {
    for (auto& account: accounts) {
        if (account.name == name) {
            if (UINT32_MAX - account.gold < amount) {
                return false;  // overflow
            }
            account.gold += amount;
            return true;
        }
    }
    return false;
}

uint32_t Banker::withdrawGold(const std::string& name, uint32_t amount) {
    for (auto& account: accounts) {
        if (account.name == name) {
            if (account.gold < amount) {
                return 0;  // insufficient funds
            }
            account.gold -= amount;
            return amount;
        }
    }
    return 0; // account not found
}

bool Banker::depositItem(const std::string& name, uint8_t itemId) {
    for (auto& account: accounts) {
        if (account.name == name) {
            for (size_t i = 0; i < INVENTORY_SIZE; ++i) {
                if (account.items[i] == 0) {
                    account.items[i] = itemId;
                    return true;
                }
            }
            return false;  // no empty slot
        }
    }
    return false;  // account not found
}

uint8_t Banker::withdrawItem(const std::string& name, uint8_t itemId) {
    for (auto& account: accounts) {
        if (account.name == name) {
            for (size_t i = 0; i < INVENTORY_SIZE; ++i) {
                if (account.items[i] == itemId) {
                    account.items[i] = 0;
                    return itemId;
                }
            }
            return 0;  // item not found
        }
    }
    return 0; // account not found
}

uint32_t Banker::getGold(const std::string& name) const {
    for (const auto& account: accounts) {
        if (account.name == name) return account.gold;
    }
    return 0;
}

std::vector<uint8_t> Banker::getItems(const std::string& name) const {
    std::vector<uint8_t> out;
    for (const auto& account: accounts) {
        if (account.name == name) {
            for (size_t i = 0; i < INVENTORY_SIZE; i++) {
                if (account.items[i] != 0) out.push_back(account.items[i]);
            }
            return out;
        }
    }
    return out;
}

Banker::~Banker() {
    for (const auto& account: accounts) {
        parser.updateBankAccount(account);
    }
}
