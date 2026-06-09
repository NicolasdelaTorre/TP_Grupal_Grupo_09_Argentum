#ifndef NPC_H
#define NPC_H

#include <cstdint>
#include <string>

#include "../../../common/position.h"

class NPC {
    protected:
        uint16_t id;
        std::string name;
        Position position;
        uint8_t mapId;

    public:
        NPC(uint16_t id, const std::string& name, uint16_t x, uint16_t y, uint8_t mapId);

        uint16_t getId() const { return id; }
        const std::string& getName() const;

        Position getPosition() const;

        void move(Position newPosition);

        virtual ~NPC() = default;
};

#endif
