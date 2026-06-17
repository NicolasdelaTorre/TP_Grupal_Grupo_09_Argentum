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
#include "clan.h"
#include "clan_registry.h"
#include "map.h"
#include "NPC/banker.h"
#include "player.h"
#include "yaml_map_loader.h"

class Game {
public:
    // itemId reservado para "drop de oro" (no es un item real). El drop lleva el
    // monto en DroppedItemRecord::goldAmount, y al levantarlo se suma al gold
    // del jugador (no entra al inventario).
    static constexpr uint8_t GOLD_ITEM_ID = 254;

    // Definido acá arriba porque lo usan tanto miembros privados como métodos
    // públicos (pickUp/drop devuelven DropResult que lo contiene).
    struct DroppedItemRecord {
        uint16_t dropId;
        uint8_t itemId;
        int16_t x;
        int16_t y;
        uint32_t goldAmount = 0;  // 0 para items normales, >0 si itemId == GOLD_ITEM_ID
    };

private:
    Map& map;
    Position playerSpawn;  // posición de spawn que viene del YAML
    std::unordered_map<int, Player> players;
    BinaryParser parser;
    // Singleton del banco
    Banker bank;
    // Registro central de clanes
    ClanRegistry clans;
    // Catalogos compartidos por tipo de comerciante ("trader" o "priest").
    // Cargados de server/Logic/merchants.toml al inicio. Pair = (itemId, precio).
    std::unordered_map<std::string, std::vector<std::pair<uint8_t, uint32_t>>> merchantCatalog;
    // Items tirados al piso (de /tirar o drops de NPC muerto). El id es
    // auto-incremental y nunca se reusa para que el cliente pueda referirse
    // a un drop específico al levantarlo.
    uint16_t nextDropId = 1;
    std::vector<DroppedItemRecord> droppedItems;
    // Encuentra una posición libre para spawnear. Tira excepción si no hay ninguna.
    Position findSpawnPosition() const;

    // True si ningún jugador está parado en pos.
    bool isPositionFree(Position pos) const;


    bool tryEvade(int attackerId, int targetId) const;

    void checkEntry(int playerId);

public:
    explicit Game(Map& world);

    // True si el nombre ya tiene datos en el binario de persistencia.
    bool playerExistsInRecords(const std::string& name);

    // Crea un jugador nuevo con la raza/clase elegidas y lo persiste.
    bool addNewPlayer(int playerId, const std::string& name, RaceCode race, ClassCode class_);

    // Carga un jugador desde el binario (ignora cualquier raza/clase).
    bool loadExistingPlayer(int playerId, const std::string& name);

    // Mueve un casillero en la dirección indicada. False si está bloqueado.
    bool movePlayer(int playerId, MoveDirection direction);

    // Gira sin moverse.
    bool turnPlayer(int playerId, MoveDirection direction);

    bool processMovement(int playerId, const std::string& direction);

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

    uint8_t getPlayerMapId(int playerId) const;

    bool hasPlayer(int playerId) const;

    // True si el jugador está muerto (fantasma).
    bool isPlayerGhost(int playerId) const;

    // Devuelve el playerId que está en (x, y, mapId), o -1 si no hay player.
    int getPlayerIdAt(int16_t x, int16_t y, uint8_t mapId) const;

    std::vector<int> getPlayerIds() const;

    void updatePlayerData(int playerId);

    void setSkin(int playerId, uint8_t bodySkinId, uint8_t headSkinId);

    uint8_t getPlayerHead(int playerId) const;

    // Resumen de lo que pasó en un ataque. Lo arma processAttack y lo usa
    // gameloop para decidir broadcasts y notificaciones al chat.
    struct AttackOutcome {
        bool valid = false;  // false si no se ejecutó (sin arma, fuera de rango...)
        std::shared_ptr<AttackResultEvent> event;
        uint16_t damage = 0;
        bool evaded = false;
        bool critical = false;
        bool killed = false;
        bool leveledUp = false;
        uint8_t newLevel = 0;  // solo válido si leveledUp == true
        std::string attackerName;
        std::string targetName;
        int attackerId = -1;
        int targetId = -1;       // id del player atacado o -1 si era NPC
        uint8_t targetType = 0;  // 0 = player, 1 = npc
        // Si fair play bloqueo el ataque, mensaje listo para enviar al chat del atacante.
        std::string blockedReason;
    };

    // Resuelve un ataque del playerId contra un target. La lógica de validar
    // arma equipada, alcance (ranged vs adyacencia melee) y daño vive adentro.
    // targetType: 0 = player, 1 = npc.
    AttackOutcome processAttack(int playerId, uint8_t targetType, uint16_t targetId);

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

    // Al morir un jugador: tira al piso todo el inventario + el oro en exceso
    // (data.gold - safeGold(level)). Devuelve los DroppedItemRecord creados
    // para que gameloop pueda broadcastear ItemDroppedEvent por cada uno.
    std::vector<DroppedItemRecord> dropPlayerLootOnDeath(int playerId);

    // Al morir una creature: tira los dados segun el enunciado (80% nada, 8%
    // oro, 1% pocion, 1% objeto). Devuelve los DroppedItemRecord creados.
    std::vector<DroppedItemRecord> dropCreatureLootOnDeath(uint16_t npcId);

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

    uint16_t applyNPCAttack(uint8_t playerId, uint16_t rawDamage);

    bool checkIfPlayerIsMeditating(int playerId) const;

    bool checkIfPlayerIsTeleporting(int playerId) const;

    void restorePlayerManaForMeditation(int playerId);

    void finishTeleportingState(int playerId);

    bool startPlayerResurrect(int playerId);

    bool lowerHealth(int playerId);

    bool lowerMana(int playerId);

    void restorePlayerHealth(int playerId);

    void restorePlayerMana(int playerId);

    void fastTravel(int playerId, Position newPosition);

    void removePlayer(int playerId);

    // ── Clanes ──────────────────────────────────────────────────────────
    // Cada try* delega en ClanRegistry y devuelve un mensaje listo para chat.
    ClanOutcome tryFoundClan(int playerId, const std::string& clanName);
    ClanOutcome tryRequestJoinClan(int playerId, const std::string& clanName);
    ClanOutcome tryAcceptClanRequest(int playerId, const std::string& applicantNick);
    ClanOutcome tryRejectClanRequest(int playerId, const std::string& applicantNick);
    ClanOutcome tryBanFromClan(int playerId, const std::string& targetNick);
    ClanOutcome tryKickFromClan(int playerId, const std::string& targetNick);
    ClanOutcome tryLeaveClan(int playerId);

    // Lineas para mostrar en /revisar-clan. Vacio si el jugador no es fundador.
    std::vector<std::string> reviewClan(int playerId) const;

    // Devuelve el nombre del clan del jugador, vacio si no esta en ninguno.
    std::string getClanOf(int playerId) const;
    // Lista de miembros del clan (nicks).
    std::vector<std::string> getClanMembers(const std::string& clanName) const;
    bool areInSameClan(int playerA, int playerB) const;
    // Cuenta miembros del mismo clan conectados dentro del radio (excluyendo al propio).
    int countNearbyClanMates(int playerId) const;

    ~Game();
};

#endif
