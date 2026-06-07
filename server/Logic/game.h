#ifndef GAME_H
#define GAME_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../common/Communication/events/server_events.h"
#include "../../common/Communication/move_direction.h"
#include "../../common/position.h"

#include "binary_parser.h"
#include "map.h"
#include "player.h"
#include "yaml_map_loader.h"

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

    // Stub de evasión. TODO(team-gameplay): implementar fórmula real con
    // dexterity del atacante vs defensor. Hoy retorna false (nunca evade).
    bool tryEvade(int attackerId, int targetId) const;

    void checkEntry(int playerId);

public:
    explicit Game(Map& world);

    // Da de alta un jugador (nuevo o cargado del binario).
    bool addPlayer(int playerId, const std::string& name, const std::string& race,
                   const std::string& class_);

    // Mueve un casillero en la dirección indicada. False si está bloqueado.
    bool movePlayer(int playerId, MoveDirection direction);

    // Gira sin moverse.
    bool turnPlayer(int playerId, MoveDirection direction);

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

    void setSkin(int playerId, uint8_t skinId);

    // Resuelve un ataque del playerId contra un target.
    // Devuelve directamente el AttackResultEvent listo para broadcast, o
    // nullptr si el ataque no se ejecutó (sin arma, no hay target en línea
    // de vista, atacante muerto, etc.). La lógica de validar arma equipada,
    // alcance (ranged vs adyacencia melee) y daño vive adentro.
    // targetType: 0 = player, 1 = npc.
    std::shared_ptr<AttackResultEvent> processAttack(int playerId, uint8_t targetType,
                                                    uint16_t targetId);

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

    bool applyNPCAttack(uint8_t playerId, uint16_t damage);

    void removePlayer(int playerId);

    ~Game();
};

#endif
