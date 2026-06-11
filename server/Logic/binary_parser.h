#ifndef BINARY_PARSER_H
#define BINARY_PARSER_H

#include <string>

#include "player.h"

struct BankAccount;

class BinaryParser {
private:
    uint32_t lastOffset;

public:
    BinaryParser();

    bool checkPlayerExists(const std::string& name);

    void savePlayerData(const std::string& name, PlayerData data);

    PlayerData loadPlayerData(const std::string& name);

    void updatePlayerData(const std::string& name, PlayerData data);

    void saveBankAccount(const BankAccount& account);

    BankAccount loadBankAccount(const std::string& name);

    bool checkBankAccountExists(const std::string& name);

    void updateBankAccount(BankAccount account);
};

#endif
