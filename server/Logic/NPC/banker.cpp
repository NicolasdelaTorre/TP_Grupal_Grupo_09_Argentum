#include "banker.h"

Banker::Banker(BinaryParser& parser): parser(parser) {}

void Banker::addPlayer(const std::string& name) {
    if (parser.checkBankAccountExists(name)) {
        BankAccount account = parser.loadBankAccount(name);
        accounts.push_back(std::move(account));
    } else {
        BankAccount newAccount{name, 0, {}};
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
            for (size_t i = 0; i < N; ++i) {
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
            for (size_t i = 0; i < N; ++i) {
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

Banker::~Banker() {
    for (const auto& account: accounts) {
        parser.updateBankAccount(account);
    }
}
