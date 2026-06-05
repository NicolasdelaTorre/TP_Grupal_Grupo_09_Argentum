#ifndef GAME_H
#define GAME_H

#include <string>
#include <unordered_map>
#include <vector>

#include "../../common/position.h"

#include "binary_parser.h"
#include "map.h"
#include "player.h"

// Resultado de un ataque, lo arma processAttack y lo consume el gameloop
// para mandar ATTACK_RESULT por broadcast.
//   performed=false  → no hubo víctima en línea de vista, no se notifica nada.
//   performed=true   → hubo víctima; hit indica si pegó o si evadió.
struct AttackResult {
    bool performed = false;
    uint16_t attackerId = 0;
    uint8_t targetType = 0;  // 0=player, 1=npc
    uint16_t targetId = 0;
    uint16_t damage = 0;
    bool hit = false;
};

class Game {
private:
    Map& map;
    Position playerSpawn;  // posición de spawn que viene del YAML
    std::unordered_map<int, Player> players;
    BinaryParser parser;

    // Encuentra una posición libre para spawnear. Tira excepción si no hay ninguna.
    Position findSpawnPosition() const;

    // True si ningún jugador está parado en pos.
    bool isPositionFree(Position pos) const;

    bool processUser(int playerId, const std::string& user);

    bool processMovement(int playerId, const std::string& direction);

    bool turnPlayer(int playerId, const std::string& direction);

    bool processHeal(int playerId);

    // Stub de evasión. TODO(team-gameplay): implementar fórmula real con
    // dexterity del atacante vs defensor. Hoy retorna false (nunca evade).
    bool tryEvade(int attackerId, int targetId) const;

public:
    Game(Map& map, Position playerSpawn);

    bool processCommand(int playerId, const std::string& command);

    Position getPlayerPosition(int playerId) const;

    const std::string& getPlayerName(int playerId) const;

    uint8_t getPlayerDirection(int playerId) const;

    uint8_t getPlayerSkin(int playerId) const;

    uint16_t getPlayerHealth(int playerId) const;

    uint16_t getPlayerMaxHealth(int playerId) const;

    uint8_t getPlayerLevel(int playerId) const;

    bool hasPlayer(int playerId) const;

    std::vector<int> getPlayerIds() const;

    void updatePlayerData(int playerId);

    void setSkin(int playerId, const std::string& skinId);

    // Resuelve un ataque desde playerId en la dirección dada. La lógica de
    // sight, daño y evasión vive adentro; el gameloop solo arma el broadcast
    // a partir del AttackResult.
    AttackResult processAttack(int playerId, const std::string& direction);

    // Resuelve un ataque dirigido a un target específico (para ranged/magia).
    // targetType: 0 = player, 1 = npc. El server valida que el target exista,
    // esté vivo y esté en rango del arma equipada antes de aplicar el daño.
    AttackResult processTargetedAttack(int playerId, uint8_t targetType, uint16_t targetId);

    // Aplica un cheat al jugador. code mapea al enum CheatCode (common/DTOs.h):
    // 0 = SUICIDE, 1 = GOLD, 2 = EXPERIENCE.
    // TODO(team-gameplay): implementar la lógica concreta (matar al jugador,
    // sumar oro, sumar experiencia). Hoy es un stub que solo loggea.
    void processCheat(int playerId, uint8_t code);

    void removePlayer(int playerId);

    ~Game();
};

#endif
