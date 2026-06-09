#ifndef GAME_H
#define GAME_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../common/Communication/events/server_events.h"
#include "../../common/DTOs.h"
#include "../../common/position.h"

#include "binary_parser.h"
#include "map.h"
#include "NPC/banker.h"
#include "player.h"
#include "yaml_map_loader.h"

class Game {
public:
    // Definido acá arriba porque lo usan tanto miembros privados como métodos
    // públicos (pickUp/drop devuelven DropResult que lo contiene).
    struct DroppedItemRecord {
        uint16_t dropId;
        uint8_t itemId;
        int16_t x;
        int16_t y;
    };

private:
    Map& map;
    Position playerSpawn;  // posición de spawn que viene del YAML
    std::unordered_map<int, Player> players;
    BinaryParser parser;
    // Singleton del banco
    Banker bank;
    // Items tirados al piso (de /tirar o drops de NPC muerto). El id es
    // auto-incremental y nunca se reusa para que el cliente pueda referirse
    // a un drop específico al levantarlo.
    uint16_t nextDropId = 1;
    std::vector<DroppedItemRecord> droppedItems;
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
    bool addPlayer(int playerId, const std::string& name, RaceCode race, ClassCode class_);

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

    // True si el jugador está muerto (fantasma).
    bool isPlayerGhost(int playerId) const;

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

    // ── Cheats invocables desde el chat (/vidainf, /gold, etc.)
    bool cheatToggleInfiniteHealth(int playerId);
    bool cheatToggleInfiniteMana(int playerId);
    bool cheatSuicide(int playerId);
    bool cheatLevelUp(int playerId);
    bool cheatAddGold(int playerId, uint32_t amount);
    bool cheatSpawnItem(int playerId, uint8_t itemId);

    // ── Interacción con NPCs amigos ─────────────────────────────────────
    // Resultado simple: ok + mensaje legible para mostrarle al jugador en el chat del sistema. Los stubs solo loguean hay que meter la lógica real usando las clases Merchant/Banker/Priest que ya existen.
    struct InteractionResult {
        bool ok = false;
        std::string message;
    };

    // /listar dirigido a un merchant: items que vende [{id, name, price}].
    // Dirigido a un banker: items y oro guardados en la cuenta del jugador.
    // Devuelve líneas de texto listas para mostrar (1 por item).
    std::vector<std::string> listMerchantInventory(uint8_t npcType);
    std::vector<std::string> listBankAccount(int playerId, uint8_t npcType);

    // /comprar <item>: merchant o priest (priest vende hechizos/pociones).
    InteractionResult buyFromNpc(int playerId, uint8_t npcType, const std::string& itemName);

    // /vender <item>: solo merchant.
    InteractionResult sellToNpc(int playerId, uint8_t npcType, const std::string& itemName);

    // /depositar <item> o /depositar oro <N>: solo banker.
    InteractionResult depositItemToBank(int playerId, const std::string& itemName);
    InteractionResult depositGoldToBank(int playerId, uint32_t amount);

    // /retirar <item> o /retirar oro <N>: solo banker.
    InteractionResult withdrawItemFromBank(int playerId, const std::string& itemName);
    InteractionResult withdrawGoldFromBank(int playerId, uint32_t amount);

    // /resucitar (priest): solo si el jugador es fantasma.
    InteractionResult revivePlayer(int playerId);

    // /curar (priest): recupera vida y maná.
    InteractionResult healPlayer(int playerId);

    // /meditar: arranca meditación (recupera mana con el tiempo). Sin NPC.
    InteractionResult meditatePlayer(int playerId);

    // ── Items en el suelo ───────────────────────────────────────────────
    const std::vector<DroppedItemRecord>& getDroppedItems() const { return droppedItems; }

    // Resultado de pickUp/drop con el record afectado para que gameloop
    // pueda armar el ItemDroppedEvent/ItemPickedUpEvent.
    struct DropResult {
        bool ok = false;
        std::string message;
        DroppedItemRecord record;
    };

    // /tomar: si hay un drop en la celda del jugador, lo agrega al inventario.
    DropResult pickUpItemAt(int playerId);

    // /tirar <slot>: saca el item del inventario y lo deja en la celda del jugador.
    DropResult dropItem(int playerId, uint8_t invSlot);

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

    bool checkIfPlayerIsMeditating(int playerId) const;

    void restorePlayerManaForMeditation(int playerId);

    void removePlayer(int playerId);

    ~Game();
};

#endif
