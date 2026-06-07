#include "npc.h"

NPC::NPC(uint16_t id, const std::string& name, uint16_t x, uint16_t y) : id(id), name(name) {
    position.x = x;
    position.y = y;
}
