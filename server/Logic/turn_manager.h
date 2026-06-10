#ifndef TURN_MANAGER_H
#define TURN_MANAGER_H

#include <unordered_map>
#include <cstdint>
#include <vector>

#define TIME 30

#include "map.h"
#include "player.h"
#include "game.h"

struct NPCTimer {
    int timeToMove; // 600 miliseconds to move
    int timeToAttack; // 1500 miliseconds to attack
    int timeToReborn; // 30000 miliseconds to reborn
};

struct PlayerTimer {
    int timeToRestoreManaMeditating; // 1000 miliseconds while the player is meditating
    int timeToTeleport; // Depends on the distance to the city
    int currentTimeToTeleport; // Time that the player has been teleporting
    int timeToRestoreHealth;
    int timeToRestoreMana;
};

class TurnManager {
    private:
        std::unordered_map<uint16_t, NPCTimer> npcTimers; // NPC ID -> NPCTimer
        std::unordered_map<uint8_t, PlayerTimer> playerTimers; // Player ID -> PlayerTimer
        Map& map;
        Game& game;

    public:
        TurnManager(std::vector<int> players, std::vector<uint16_t> npcIds, Map& map, Game& game);

        void addPlayers(std::vector<int> playerIds);

        void removePlayers(std::vector<int> playerIds);

        void updateTimers();

        std::vector<int> getPlayersReadyToRestoreHealth();

        std::vector<int> getPlayersReadyToRestoreManaThroughTime();

        std::vector<int> getPlayersReadyToRestoreManaByMeditation();

        bool alreadyTeleporting(int playerId);

        void setTimeToTeleport(int playerId, int timeToTeleport);

        std::vector<int> getPlayersReadyToTeleport();

        // Return all NPCs that are ready to move or attack, and reset their timers.
        std::vector<uint16_t> getNPCsReady(bool forMove);

        std::vector<uint16_t> reviveNPCs();
};

#endif
