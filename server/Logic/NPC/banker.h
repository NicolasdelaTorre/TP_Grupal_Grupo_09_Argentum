#ifndef BANKER_H
#define BANKER_H

#include <cstdint>
#include <string>
#include <vector>

#include "../binary_parser.h"
#include "npc.h"

#define N 9

struct BankAccount {
    std::string name;
    uint32_t gold;
    uint8_t items[N];
};

class Banker : public NPC {
    private:
        std::vector<BankAccount> accounts;
        BinaryParser parser;

    public:
        Banker(uint16_t id, const std::string& name, uint16_t x, uint16_t y);

        void addPlayer(const std::string& name);

        bool depositGold(const std::string& name, uint32_t amount);

        uint32_t withdrawGold(const std::string& name, uint32_t amount);

        bool depositItem(const std::string& name, uint8_t itemId);

        uint8_t withdrawItem(const std::string& name, uint8_t itemId);

        // Lectura del estado de una cuenta. Devuelve 0 si no existe.
        uint32_t getGold(const std::string& name) const;

        // Devuelve los itemIds guardados (sin slots vacios). Vacio si no existe.
        std::vector<uint8_t> getItems(const std::string& name) const;

        ~Banker() override;
};

#endif
