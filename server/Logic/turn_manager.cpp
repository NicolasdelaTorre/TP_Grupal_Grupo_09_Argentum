#include "turn_manager.h"

TurnManager::TurnManager(std::vector<uint16_t> npcIds, Map& map) : map(map) {
    for (uint16_t npcId : npcIds) {
        if (map.isACreature(npcId)) {
            npcTimers[npcId] = {0, 0, 0};
        }
    }
}

void TurnManager::addNPC(uint16_t npcId) {
    if (map.isACreature(npcId)) {
        npcTimers[npcId] = {0, 0, 0};
    }
}

void TurnManager::removeNPC(uint16_t npcId) {
    npcTimers.erase(npcId);
}

void TurnManager::updateTimers() {
    for (auto& pair : npcTimers) {
        uint16_t npcId = pair.first;
        NPCTimer& timer = pair.second;

        if (map.checkNPCAlive(npcId)) {
            if (map.checkIfNPCIsNextToAPlayer(npcId, map.getNPC(npcId)->getMapId())) {
                timer.timeToAttack += TIME;
                timer.timeToMove = 0; // reset move timer if next to player
            } else {
                timer.timeToMove += TIME;
                timer.timeToAttack = 0; // reset attack timer if not next to player
            }
        } else {
            timer.timeToReborn += TIME;
            timer.timeToMove = 0;
            timer.timeToAttack = 0;
        }
    }
}

std::vector<uint16_t> TurnManager::getNPCsReady(bool forMove) {
    std::vector<uint16_t> readyNPCs;
    for (auto& pair : npcTimers) {
        uint16_t npcId = pair.first;
        const NPCTimer& timer = pair.second;

        if (!map.checkNPCAlive(npcId)) continue;

        if (forMove && timer.timeToMove >= 600) {
            readyNPCs.push_back(npcId);
            // Reset move timer after moving
            npcTimers[npcId].timeToMove = 0;
        }

        if (!forMove && timer.timeToAttack >= 1500) {
            readyNPCs.push_back(npcId);
            // Reset attack timer after attacking
            npcTimers[npcId].timeToAttack = 0;
        }
    }
    return readyNPCs;
}

std::vector<uint16_t> TurnManager::reviveNPCs() {
    std::vector<uint16_t> revivedNPCs;
    for (auto& pair : npcTimers) {
        uint16_t npcId = pair.first;
        NPCTimer& timer = pair.second;

        if (!map.checkNPCAlive(npcId) && timer.timeToReborn >= 30000) {
            revivedNPCs.push_back(npcId);
            // Reset reborn timer after reviving
            npcTimers[npcId].timeToReborn = 0;
        }
    }
    return revivedNPCs;
}
