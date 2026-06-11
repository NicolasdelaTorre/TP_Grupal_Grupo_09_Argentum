#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_SERVER_EVENTS_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_SERVER_EVENTS_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "server_event.h"

// Datos de una celda del mapa, en el formato del wire.
struct MapCellData {
    uint16_t textureId;
    uint16_t obstacleId;
    bool safeZone;
};

// Obstáculo colocado en el mapa. (x, y) es la esquina superior-izquierda del
// rectángulo que bloquea (footprint), y (w, h) su tamaño en tiles. El cliente
// dibuja la textura del tipo a tamaño nativo anclada a la esquina inferior
// izquierda, es independiente del tamaño que bloquea.
struct MapObstacleData {
    uint8_t type;
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
};

// Eventos sin payload (solo el opcode). Sirve para LOGIN_FAIL, FIRST_LOGIN.
class OpcodeOnlyEvent: public ServerEvent {
private:
    uint8_t opcode;

public:
    explicit OpcodeOnlyEvent(uint8_t opcode);
    uint8_t getOpcode() const { return opcode; }
    void serialize(CommonProtocol& proto) const override;
    // El opcode se conoce al hacer el switch en deserialize → no se necesita
    // un deserialize para esta clase (la factory construye el evento direct).
};

// LOGIN_OK: [opcode][spawn_x:2][spawn_y:2].
class LoginOkEvent: public ServerEvent {
private:
    int16_t spawnX;
    int16_t spawnY;
    uint8_t skin;
    uint8_t head;

public:
    LoginOkEvent(int16_t spawnX, int16_t spawnY, uint8_t skin, uint8_t head);
    int16_t getSpawnX() const { return spawnX; }
    int16_t getSpawnY() const { return spawnY; }
    uint8_t getSkin() const { return skin; }
    uint8_t getHead() const { return head; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<LoginOkEvent> deserialize(CommonProtocol& proto);
};

// MAP: [opcode][width:2][height:2][cellCount:2][cells...]
//      [obstacleCount:2][[type:1][x:2][y:2][w:2][h:2]...].
class MapEvent: public ServerEvent {
private:
    uint16_t width;
    uint16_t height;
    std::vector<MapCellData> cells;
    std::vector<MapObstacleData> obstacles;

public:
    MapEvent(uint16_t width, uint16_t height, std::vector<MapCellData> cells,
             std::vector<MapObstacleData> obstacles = {});
    uint16_t getWidth() const { return width; }
    uint16_t getHeight() const { return height; }
    const std::vector<MapCellData>& getCells() const { return cells; }
    const std::vector<MapObstacleData>& getObstacles() const { return obstacles; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<MapEvent> deserialize(CommonProtocol& proto);
};

// NEW_PLAYER: [opcode][id:2][x:2][y:2][dir:1][skin:1][head:1][name_len:2][name:n].
class NewPlayerEvent: public ServerEvent {
private:
    uint16_t id;
    int16_t x;
    int16_t y;
    uint8_t dir;
    uint8_t skin;
    uint8_t head;
    std::string name;

public:
    NewPlayerEvent(uint16_t id, int16_t x, int16_t y, uint8_t dir, uint8_t skin, uint8_t head,
                   std::string name);
    uint16_t getId() const { return id; }
    int16_t getX() const { return x; }
    int16_t getY() const { return y; }
    uint8_t getDir() const { return dir; }
    uint8_t getSkin() const { return skin; }
    uint8_t getHead() const { return head; }
    const std::string& getName() const { return name; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<NewPlayerEvent> deserialize(CommonProtocol& proto);
};

// MOVE_REJECTED: [opcode][x:2][y:2]. El server rechazo la prediccion del
// cliente y le manda la posicion autoritativa para que reconcilie. Pensado
// para los casos race-condition donde el cliente ya predijo un move que el
// server justo rechazo (NPC moviendose al mismo tile, etc).
class MoveRejectedEvent: public ServerEvent {
private:
    int16_t x;
    int16_t y;

public:
    MoveRejectedEvent(int16_t x, int16_t y);
    int16_t getX() const { return x; }
    int16_t getY() const { return y; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<MoveRejectedEvent> deserialize(CommonProtocol& proto);
};

// PLAYER_MOVED: [opcode][id:2][x:2][y:2][dir:1]. También sirve para TURN.
class PlayerMovedEvent: public ServerEvent {
private:
    uint16_t id;
    int16_t x;
    int16_t y;
    uint8_t dir;

public:
    PlayerMovedEvent(uint16_t id, int16_t x, int16_t y, uint8_t dir);
    uint16_t getId() const { return id; }
    int16_t getX() const { return x; }
    int16_t getY() const { return y; }
    uint8_t getDir() const { return dir; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<PlayerMovedEvent> deserialize(CommonProtocol& proto);
};

// PLAYER_DISCONNECTED: [opcode][id:2].
class PlayerDisconnectedEvent: public ServerEvent {
private:
    uint16_t id;

public:
    explicit PlayerDisconnectedEvent(uint16_t id);
    uint16_t getId() const { return id; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<PlayerDisconnectedEvent> deserialize(CommonProtocol& proto);
};

// STATS_JUGADOR: [opcode][hp:2][maxHp:2][mana:2][maxMana:2]
//                [gold:4][exp:4][nextLvlExp:4][level:1].
class StatsEvent: public ServerEvent {
private:
    uint16_t hp;
    uint16_t maxHp;
    uint16_t mana;
    uint16_t maxMana;
    uint32_t gold;
    uint32_t exp;
    uint32_t nextLvlExp;
    uint8_t level;

public:
    StatsEvent(uint16_t hp, uint16_t maxHp, uint16_t mana, uint16_t maxMana, uint32_t gold,
               uint32_t exp, uint32_t nextLvlExp, uint8_t level);
    uint16_t getHp() const { return hp; }
    uint16_t getMaxHp() const { return maxHp; }
    uint16_t getMana() const { return mana; }
    uint16_t getMaxMana() const { return maxMana; }
    uint32_t getGold() const { return gold; }
    uint32_t getExp() const { return exp; }
    uint32_t getNextLvlExp() const { return nextLvlExp; }
    uint8_t getLevel() const { return level; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<StatsEvent> deserialize(CommonProtocol& proto);
};

// ATTACK_RESULT: [opcode][attackerId:2][targetType:1][targetId:2][damage:2][hit:1].
class AttackResultEvent: public ServerEvent {
private:
    uint16_t attackerId;
    uint8_t targetType;  // 0 = player, 1 = npc
    uint16_t targetId;
    uint16_t damage;
    bool hit;

public:
    AttackResultEvent(uint16_t attackerId, uint8_t targetType, uint16_t targetId, uint16_t damage,
                      bool hit);
    uint16_t getAttackerId() const { return attackerId; }
    uint8_t getTargetType() const { return targetType; }
    uint16_t getTargetId() const { return targetId; }
    uint16_t getDamage() const { return damage; }
    bool getHit() const { return hit; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<AttackResultEvent> deserialize(CommonProtocol& proto);
};

// INVENTORY_UPDATE: [opcode][count:1][[itemId:1]...×count][eqW:1][eqA:1][eqH:1][eqS:1].
class InventoryUpdateEvent: public ServerEvent {
private:
    std::vector<uint8_t> items;
    uint8_t equippedWeapon;
    uint8_t equippedArmor;
    uint8_t equippedHelmet;
    uint8_t equippedShield;

public:
    InventoryUpdateEvent(std::vector<uint8_t> items, uint8_t equippedWeapon, uint8_t equippedArmor,
                         uint8_t equippedHelmet, uint8_t equippedShield);
    const std::vector<uint8_t>& getItems() const { return items; }
    uint8_t getEquippedWeapon() const { return equippedWeapon; }
    uint8_t getEquippedArmor() const { return equippedArmor; }
    uint8_t getEquippedHelmet() const { return equippedHelmet; }
    uint8_t getEquippedShield() const { return equippedShield; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<InventoryUpdateEvent> deserialize(CommonProtocol& proto);
};

// PLAYER_EQUIPPED: [opcode][playerId:2][slot:1][itemId:1].
class PlayerEquippedEvent: public ServerEvent {
private:
    uint16_t playerId;
    uint8_t slot;  // 0=arma, 1=armor, 2=casco, 3=escudo
    uint8_t itemId;

public:
    PlayerEquippedEvent(uint16_t playerId, uint8_t slot, uint8_t itemId);
    uint16_t getPlayerId() const { return playerId; }
    uint8_t getSlot() const { return slot; }
    uint8_t getItemId() const { return itemId; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<PlayerEquippedEvent> deserialize(CommonProtocol& proto);
};

// NEW_NPC: [opcode][id:2][x:2][y:2][type:1][alive:1].
class NewNpcEvent: public ServerEvent {
private:
    uint16_t id;
    int16_t x;
    int16_t y;
    uint8_t type;
    bool alive;

public:
    NewNpcEvent(uint16_t id, int16_t x, int16_t y, uint8_t type, bool alive);
    uint16_t getId() const { return id; }
    int16_t getX() const { return x; }
    int16_t getY() const { return y; }
    uint8_t getType() const { return type; }
    bool getAlive() const { return alive; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<NewNpcEvent> deserialize(CommonProtocol& proto);
};

// NPC_MOVED: [opcode][id:2][x:2][y:2][dir:1].
class NpcMovedEvent: public ServerEvent {
private:
    uint16_t id;
    int16_t x;
    int16_t y;
    uint8_t dir;

public:
    NpcMovedEvent(uint16_t id, int16_t x, int16_t y, uint8_t dir);
    uint16_t getId() const { return id; }
    int16_t getX() const { return x; }
    int16_t getY() const { return y; }
    uint8_t getDir() const { return dir; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<NpcMovedEvent> deserialize(CommonProtocol& proto);
};

// NPC_DIED: [opcode][id:2].
class NpcDiedEvent: public ServerEvent {
private:
    uint16_t id;

public:
    explicit NpcDiedEvent(uint16_t id);
    uint16_t getId() const { return id; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<NpcDiedEvent> deserialize(CommonProtocol& proto);
};

// NPC_RESPAWNED: [opcode][id:2][x:2][y:2].
class NpcRespawnedEvent: public ServerEvent {
private:
    uint16_t id;
    int16_t x;
    int16_t y;

public:
    NpcRespawnedEvent(uint16_t id, int16_t x, int16_t y);
    uint16_t getId() const { return id; }
    int16_t getX() const { return x; }
    int16_t getY() const { return y; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<NpcRespawnedEvent> deserialize(CommonProtocol& proto);
};

// ITEM_DROPPED: [opcode][drop_id:2][item_id:1][x:2][y:2][gold_amount:4].
// Aparece un item en el piso. dropId es la clave que despues se usa para
// sacarlo del piso con ItemPickedUpEvent. Tanto /tirar, NPC muerto, o player
// muerto disparan este evento.
// itemId=254 (GOLD_ITEM_ID) es un caso especial: el drop es oro y el monto
// viaja en goldAmount. Para items normales goldAmount=0.
class ItemDroppedEvent: public ServerEvent {
private:
    uint16_t dropId;
    uint8_t itemId;
    int16_t x;
    int16_t y;
    uint32_t goldAmount;

public:
    ItemDroppedEvent(uint16_t dropId, uint8_t itemId, int16_t x, int16_t y,
                     uint32_t goldAmount = 0);
    uint16_t getDropId() const { return dropId; }
    uint8_t getItemId() const { return itemId; }
    int16_t getX() const { return x; }
    int16_t getY() const { return y; }
    uint32_t getGoldAmount() const { return goldAmount; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<ItemDroppedEvent> deserialize(CommonProtocol& proto);
};

// ITEM_PICKED_UP: [opcode][drop_id:2]. El cliente saca ese dropId del piso.
class ItemPickedUpEvent: public ServerEvent {
private:
    uint16_t dropId;

public:
    explicit ItemPickedUpEvent(uint16_t dropId);
    uint16_t getDropId() const { return dropId; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<ItemPickedUpEvent> deserialize(CommonProtocol& proto);
};

// PLAYER_DIED: [opcode][id:2]. El cliente cambia el sprite a fantasma.
class PlayerDiedEvent: public ServerEvent {
private:
    uint16_t id;

public:
    explicit PlayerDiedEvent(uint16_t id);
    uint16_t getId() const { return id; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<PlayerDiedEvent> deserialize(CommonProtocol& proto);
};

// PLAYER_REVIVED: [opcode][id:2][x:2][y:2]. La posición sirve para el caso de
// /resucitar con teletransporte al sacerdote más cercano.
class PlayerRevivedEvent: public ServerEvent {
private:
    uint16_t id;
    int16_t x;
    int16_t y;

public:
    PlayerRevivedEvent(uint16_t id, int16_t x, int16_t y);
    uint16_t getId() const { return id; }
    int16_t getX() const { return x; }
    int16_t getY() const { return y; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<PlayerRevivedEvent> deserialize(CommonProtocol& proto);
};

// CHAT_MSG: [opcode][author_id:2][name_len:2][name][msg_len:2][msg].
// author_id=0 indica mensaje del sistema (name vacío).
class ChatBroadcastEvent: public ServerEvent {
private:
    uint16_t authorId;
    std::string authorName;
    std::string text;

public:
    ChatBroadcastEvent(uint16_t authorId, std::string authorName, std::string text);
    uint16_t getAuthorId() const { return authorId; }
    const std::string& getAuthorName() const { return authorName; }
    const std::string& getText() const { return text; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<ChatBroadcastEvent> deserialize(CommonProtocol& proto);
};

#endif
