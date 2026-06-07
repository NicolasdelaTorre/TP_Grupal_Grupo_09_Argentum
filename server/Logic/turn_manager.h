#ifndef TURN_MANAGER_H
#define TURN_MANAGER_H

#include <unordered_map>
#include <cstdint>
#include <vector>

#define TIME 30

#include "map.h"

struct NPCTimer {
    int timeToMove; // 600 miliseconds to move
    int timeToAttack; // 1500 miliseconds to attack
    int timeToReborn; // 30000 miliseconds to reborn
};

class TurnManager {
    private:
        std::unordered_map<uint16_t, NPCTimer> npcTimers; // NPC ID -> NPCTimer
        Map& map;

    public:
        TurnManager(std::vector<uint16_t> npcIds, Map& map);

        void addNPC(uint16_t npcId);

        void removeNPC(uint16_t npcId);

        void updateTimers();

        // Return all NPCs that are ready to move or attack, and reset their timers.
        std::vector<uint16_t> getNPCsReady(bool forMove);

        std::vector<uint16_t> reviveNPCs();
};

#endif
