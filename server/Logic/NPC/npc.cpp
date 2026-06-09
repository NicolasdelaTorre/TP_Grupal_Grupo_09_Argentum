#include "npc.h"

NPC::NPC(uint16_t id, const std::string& name, uint16_t x, uint16_t y, uint8_t mapId) : id(id), name(name), mapId(mapId) {
    position.x = x;
    position.y = y;
}

const std::string& NPC::getName() const {
    return name;
}

Position NPC::getPosition() const {
    return position;
}

void NPC::move(Position newPosition) {
    position = newPosition;
}
