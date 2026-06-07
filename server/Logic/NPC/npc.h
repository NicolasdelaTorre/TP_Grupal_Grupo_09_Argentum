#ifndef NPC_H
#define NPC_H

#include <cstdint>
#include <string>

#include "../../../common/position.h"

class NPC {
    protected:
        uint16_t id;
        const std::string& name;
        Position position;
        uint8_t mapId;

    public:
        NPC(uint16_t id, const std::string& name, uint16_t x, uint16_t y, uint8_t mapId);

        virtual ~NPC() = default;
};

#endif
