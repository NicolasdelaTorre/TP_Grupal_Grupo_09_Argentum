#include "binary_parser.h"

#include "../../common/game_constants.h"

#include "NPC/banker.h"

#include <cstdint>
#include <fstream>
#include <string>

BinaryParser::BinaryParser(): lastOffset(0) {
    std::ifstream playersFile("server/Logic/Player/players_data.bin", std::ios::binary | std::ios::ate);

    if (playersFile.is_open()) {
        lastOffset = static_cast<uint32_t>(playersFile.tellg());
    }
}

bool BinaryParser::findPlayerOffset(const std::string& name, uint32_t& offset) {
    std::ifstream playersFile("server/Logic/Player/players.bin", std::ios::binary);
    
    if (!playersFile.is_open()) return false;

    while (true) {
        uint16_t nameLength = 0;

        if (!playersFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength))) {
            break;
        }

        if (nameLength == 0) {
            break;
        }

        std::string storedName(nameLength, '\0');
        if (!playersFile.read(&storedName[0], nameLength)) {
            break;
        }

        uint32_t dataOffset = 0;
        if (!playersFile.read(reinterpret_cast<char*>(&dataOffset), sizeof(dataOffset))) {
            break;
        }

        if (storedName == name) {
            offset = dataOffset;
            return true;
        }
    }

    return false;
}

bool BinaryParser::findBankAccountOffset(const std::string& name, std::streampos& offset) {
    std::ifstream bankFile("server/Logic/NPC/bank_accounts.bin", std::ios::binary);
    
    if (!bankFile.is_open()) return false;

    while (true) {
        uint16_t nameLength = 0;

        if (!bankFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength))) {
            break;
        }

        if (nameLength == 0) {
            break;
        }

        std::string storedName(nameLength, '\0');
        if (!bankFile.read(&storedName[0], nameLength)) {
            break;
        }

        offset = bankFile.tellg();

        uint32_t dataGold = 0;
        if (!bankFile.read(reinterpret_cast<char*>(&dataGold), sizeof(dataGold))) break;

        uint8_t items[INVENTORY_SIZE];
        if (!bankFile.read(reinterpret_cast<char*>(items), sizeof(items))) break;

        if (storedName == name) {
            return true;
        }
    }

    return false;
}

bool BinaryParser::checkPlayerExists(const std::string& name) {
    uint32_t offset;
    return findPlayerOffset(name, offset);
}

void BinaryParser::savePlayerData(const std::string& name, PlayerData data) {
    uint16_t nameLength = name.size();

    std::ofstream playersFile("server/Logic/Player/players.bin", std::ios::binary | std::ios::app);
    playersFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    playersFile.write(name.c_str(), nameLength);
    playersFile.write(reinterpret_cast<const char*>(&lastOffset), sizeof(lastOffset));
    playersFile.close();

    std::ofstream dataFile("server/Logic/Player/players_data.bin", std::ios::binary | std::ios::app);
    dataFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
    lastOffset += sizeof(data);
    dataFile.close();
}

PlayerData BinaryParser::loadPlayerData(const std::string& name) {
    uint32_t offset;
    if (!findPlayerOffset(name, offset)) {
        throw std::runtime_error("BinaryParser Error: player not found");
    }

    std::ifstream playersFile("server/Logic/Player/players_data.bin", std::ios::binary);
    if (!playersFile.is_open()) {
        throw std::runtime_error("BinaryParser Error: could not open players file");
    }

    playersFile.seekg(offset);
    PlayerData data;
    playersFile.read(reinterpret_cast<char*>(&data), sizeof(data));
    return data;
}

void BinaryParser::updatePlayerData(const std::string& name, PlayerData data) {
    uint32_t offset;
    if (!findPlayerOffset(name, offset)) {
        throw std::runtime_error("BinaryParser Error: player not found");
    }
    
    std::fstream playersFile("server/Logic/Player/players_data.bin", std::ios::binary | std::ios::in | std::ios::out);
    if (!playersFile.is_open()) {
        throw std::runtime_error("BinaryParser Error: could not open players file");
    }

    playersFile.seekp(offset);
    playersFile.write(reinterpret_cast<const char*>(&data), sizeof(data));
}

void BinaryParser::saveBankAccount(const BankAccount& account) {
    std::ofstream bankFile("server/Logic/NPC/bank_accounts.bin", std::ios::binary | std::ios::app);
    if (!bankFile.is_open()) return;

    uint16_t nameLength = account.name.size();
    bankFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    bankFile.write(account.name.c_str(), nameLength);
    bankFile.write(reinterpret_cast<const char*>(&account.gold), sizeof(account.gold));
    bankFile.write(reinterpret_cast<const char*>(account.items), sizeof(account.items));
}

BankAccount BinaryParser::loadBankAccount(const std::string& name) {
    std::streampos accountOffset;
    if (!findBankAccountOffset(name, accountOffset)) {
        throw std::runtime_error("BinaryParser Error: bank account not found");
    }

    std::ifstream bankFile("server/Logic/NPC/bank_accounts.bin", std::ios::binary);
    if (!bankFile.is_open()) {
        throw std::runtime_error("BinaryParser Error: could not open bank accounts file");
    }

    bankFile.seekg(accountOffset);
    BankAccount account{name, 0, {}};
    bankFile.read(reinterpret_cast<char*>(&account.gold), sizeof(account.gold));
    bankFile.read(reinterpret_cast<char*>(account.items), sizeof(account.items));
    
    return account;
}

bool BinaryParser::checkBankAccountExists(const std::string& name) {
    std::streampos dummyOffset;
    return findBankAccountOffset(name, dummyOffset);
}

void BinaryParser::updateBankAccount(BankAccount account) {
    std::streampos accountOffset;
    if (!findBankAccountOffset(account.name, accountOffset)) {
        return; 
    }

    std::fstream dataFile("server/Logic/NPC/bank_accounts.bin", std::ios::binary | std::ios::in | std::ios::out);
    if (!dataFile.is_open()) {
        throw std::runtime_error("BinaryParser Error: could not open bank accounts file");
    }
    
    dataFile.seekp(accountOffset);
    dataFile.write(reinterpret_cast<const char*>(&account.gold), sizeof(account.gold));
    dataFile.write(reinterpret_cast<const char*>(account.items), sizeof(account.items));
}
