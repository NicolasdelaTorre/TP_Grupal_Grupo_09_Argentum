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

    uint16_t getPlayerMana(int playerId) const;

    uint16_t getPlayerMaxMana(int playerId) const;

    uint32_t getPlayerGold(int playerId) const;

    uint32_t getPlayerExperience(int playerId) const;

    // Limite = 1000 * Nivel^1.8 (sección "Puntos de experiencia" del enunciado).
    uint32_t getPlayerNextLevelExp(int playerId) const;

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

    // Recoge lo que haya en la celda del jugador (`/tomar`).
    // TODO(team-gameplay): buscar item en droppedItems en la posición del
    // jugador, llamarlo a player.addItem y removerlo del piso. Hoy stub.
    bool pickUpItemAt(int playerId);

    // Tira el item del slot al piso (`/tirar`).
    // TODO(team-gameplay): sacar de player.inventory[invSlot] y agregar a
    // droppedItems en la posición del jugador. Hoy stub.
    bool dropItem(int playerId, uint8_t invSlot);

    // Equipa o usa el item del slot según su tipo (ver ADR-002).
    // TODO(team-gameplay): si arma/armor/casco/escudo → player.equipItem(slot).
    // Si poción → consumir y aplicar efecto (heal / mana). Hoy stub.
    bool equipOrUseItem(int playerId, uint8_t invSlot);

    // Desequipa el slot indicado. slotType: 0=arma, 1=armor, 2=casco, 3=escudo.
    // TODO(team-gameplay): mapear slotType → ItemType y llamar a
    // player.unequipItem. Hoy stub.
    bool unequipSlot(int playerId, uint8_t slotType);

    // Devuelve el inventario actual del jugador para serializar INVENTORY_UPDATE.
    // {invItemIds, equippedWeaponId, equippedArmorId, equippedHelmetId, equippedShieldId}
    struct InventorySnapshot {
        std::vector<uint8_t> items;  // ids de items en el inv (sin slots vacíos)
        uint8_t equippedWeapon = 0;
        uint8_t equippedArmor = 0;
        uint8_t equippedHelmet = 0;
        uint8_t equippedShield = 0;
    };
    InventorySnapshot getInventorySnapshot(int playerId) const;

    void removePlayer(int playerId);

    ~Game();
};

#endif
