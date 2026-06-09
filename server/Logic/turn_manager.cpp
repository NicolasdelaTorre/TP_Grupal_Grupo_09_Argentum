#include "turn_manager.h"

TurnManager::TurnManager(std::vector<int> players, std::vector<uint16_t> npcIds, Map& map, Game& game) : map(map), game(game) {
    for (const auto& playerId : players) {
        playerTimers[playerId] = {0, 0, 0};
    }

    for (uint16_t npcId : npcIds) {
        if (map.isACreature(npcId)) {
            npcTimers[npcId] = {0, 0, 0};
        }
    }
}

void TurnManager::addPlayers(std::vector<int> playerIds) {
    for (int playerId : playerIds) {
        if (playerTimers.find(playerId) == playerTimers.end()) {
            playerTimers[playerId] = {0, 0, 0};
        }
    }
}

void TurnManager::removePlayers(std::vector<int> playerIds) {
    std::vector<int> toRemove;
    for (auto& pair : playerTimers) {
        int playerId = pair.first;
        if (std::find(playerIds.begin(), playerIds.end(), playerId) == playerIds.end()) {
            toRemove.push_back(playerId);
        }
    }
    
    for (int playerId : toRemove) {
        playerTimers.erase(playerId);
    }
}

void TurnManager::updateTimers() {
    // For Players
    for (auto& pair : playerTimers) {
        uint8_t playerId = pair.first;
        PlayerTimer& timer = pair.second;

        // Meditation
        if (game.checkIfPlayerIsMeditating(playerId)) {
            timer.timeToRestoreManaMeditating += TIME;
        } else {
            timer.timeToRestoreManaMeditating = 0;
        }

        // Teleport
        if (timer.timeToTeleport > 0) {
            timer.currentTimeToTeleport += TIME;
        }
    }

    // For NPCs
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

std::vector<int> TurnManager::getPlayersReadyToRestoreMana() {
    std::vector<int> readyPlayers;
    for (const auto& pair : playerTimers) {
        uint8_t playerId = pair.first;
        const PlayerTimer& timer = pair.second;

        if (timer.timeToRestoreManaMeditating >= 1000) {
            readyPlayers.push_back(playerId);
            // Reset meditation timer after restoring mana
            playerTimers[playerId].timeToRestoreManaMeditating = 0;
        }
    }
    return readyPlayers;
}

bool TurnManager::alreadyTeleporting(int playerId) {
    return playerTimers.find(playerId) != playerTimers.end() && playerTimers[playerId].timeToTeleport > 0;
}

void TurnManager::setTimeToTeleport(int playerId, int timeToTeleport) {
    if (playerTimers.find(playerId) != playerTimers.end()) {
        playerTimers[playerId].timeToTeleport = timeToTeleport;
    }
}

std::vector<int> TurnManager::getPlayersReadyToTeleport() {
    std::vector<int> readyPlayers;
    for (const auto& pair : playerTimers) {
        uint8_t playerId = pair.first;
        const PlayerTimer& timer = pair.second;

        if (timer.timeToTeleport > 0 && timer.currentTimeToTeleport >= timer.timeToTeleport) {
            readyPlayers.push_back(playerId);
            // Reset teleport timers after teleporting
            playerTimers[playerId].timeToTeleport = 0;
            playerTimers[playerId].currentTimeToTeleport = 0;
        }
    }
    return readyPlayers;
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
