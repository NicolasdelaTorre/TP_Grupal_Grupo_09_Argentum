#ifndef BANKER_H
#define BANKER_H

#include <cstdint>
#include <string>
#include <vector>

#include "../binary_parser.h"

#define N 9

struct BankAccount {
    std::string name;
    uint32_t gold;
    uint8_t items[N];
};

class Banker {
    private:
        std::vector<BankAccount> accounts;
        BinaryParser& parser;

    public:
        Banker(BinaryParser& parser);

        void addPlayer(const std::string& name);

        bool depositGold(const std::string& name, uint32_t amount);

        uint32_t withdrawGold(const std::string& name, uint32_t amount);

        bool depositItem(const std::string& name, uint8_t itemId);

        uint8_t withdrawItem(const std::string& name, uint8_t itemId);

        ~Banker();
};

#endif
