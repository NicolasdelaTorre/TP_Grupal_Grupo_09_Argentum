#include "server_events.h"

#include <stdexcept>
#include <utility>

#include "../message_types.h"

// ── ServerEvent factory ──────────────────────────────────────────────────

std::unique_ptr<ServerEvent> ServerEvent::deserialize(uint8_t opcode, CommonProtocol& proto) {
    switch (static_cast<ServerMsg>(opcode)) {
        case ServerMsg::LOGIN_OK:
            return LoginOkEvent::deserialize(proto);
        case ServerMsg::LOGIN_FAIL:
        case ServerMsg::FIRST_LOGIN:
        case ServerMsg::MOVE_OK:
        case ServerMsg::MOVE_FAIL:
            return std::make_unique<OpcodeOnlyEvent>(opcode);
        case ServerMsg::MAP:
            return MapEvent::deserialize(proto);
        case ServerMsg::NEW_PLAYER:
            return NewPlayerEvent::deserialize(proto);
        case ServerMsg::PLAYER_MOVED:
            return PlayerMovedEvent::deserialize(proto);
        case ServerMsg::PLAYER_DISCONNECTED:
            return PlayerDisconnectedEvent::deserialize(proto);
        case ServerMsg::STATS_JUGADOR:
            return StatsEvent::deserialize(proto);
        case ServerMsg::ATTACK_RESULT:
            return AttackResultEvent::deserialize(proto);
        case ServerMsg::INVENTORY_UPDATE:
            return InventoryUpdateEvent::deserialize(proto);
        case ServerMsg::PLAYER_EQUIPPED:
            return PlayerEquippedEvent::deserialize(proto);
        case ServerMsg::NEW_NPC:
            return NewNpcEvent::deserialize(proto);
        case ServerMsg::NPC_MOVED:
            return NpcMovedEvent::deserialize(proto);
        case ServerMsg::NPC_DIED:
            return NpcDiedEvent::deserialize(proto);
        case ServerMsg::NPC_RESPAWNED:
            return NpcRespawnedEvent::deserialize(proto);
        default:
            throw std::runtime_error("ServerEvent: unknown opcode");
    }
}

// ── OpcodeOnlyEvent ──────────────────────────────────────────────────────

OpcodeOnlyEvent::OpcodeOnlyEvent(uint8_t opcode): opcode(opcode) {}

void OpcodeOnlyEvent::serialize(CommonProtocol& proto) const { proto.sendByte(opcode); }

// ── LoginOkEvent ─────────────────────────────────────────────────────────

LoginOkEvent::LoginOkEvent(int16_t spawnX, int16_t spawnY): spawnX(spawnX), spawnY(spawnY) {}

void LoginOkEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::LOGIN_OK));
    proto.send_two_bytes_number(static_cast<uint16_t>(spawnX));
    proto.send_two_bytes_number(static_cast<uint16_t>(spawnY));
}

std::unique_ptr<LoginOkEvent> LoginOkEvent::deserialize(CommonProtocol& proto) {
    int16_t x = static_cast<int16_t>(proto.receive_two_bytes_number());
    int16_t y = static_cast<int16_t>(proto.receive_two_bytes_number());
    return std::make_unique<LoginOkEvent>(x, y);
}

// ── MapEvent ─────────────────────────────────────────────────────────────

MapEvent::MapEvent(uint16_t width, uint16_t height, std::vector<MapCellData> cells):
        width(width), height(height), cells(std::move(cells)) {}

void MapEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::MAP));
    proto.send_two_bytes_number(width);
    proto.send_two_bytes_number(height);
    proto.send_two_bytes_number(static_cast<uint16_t>(cells.size()));
    for (const auto& c: cells) {
        proto.send_two_bytes_number(c.textureId);
        proto.send_two_bytes_number(c.obstacleId);
        proto.sendByte(c.safeZone ? 1 : 0);
    }
}

std::unique_ptr<MapEvent> MapEvent::deserialize(CommonProtocol& proto) {
    uint16_t w = proto.receive_two_bytes_number();
    uint16_t h = proto.receive_two_bytes_number();
    uint16_t count = proto.receive_two_bytes_number();
    std::vector<MapCellData> cells;
    cells.reserve(count);
    for (uint16_t i = 0; i < count; i++) {
        MapCellData c;
        c.textureId = proto.receive_two_bytes_number();
        c.obstacleId = proto.receive_two_bytes_number();
        c.safeZone = (proto.receive_byte() != 0);
        cells.push_back(c);
    }
    return std::make_unique<MapEvent>(w, h, std::move(cells));
}

// ── NewPlayerEvent ───────────────────────────────────────────────────────

NewPlayerEvent::NewPlayerEvent(uint16_t id, int16_t x, int16_t y, uint8_t dir, uint8_t skin,
                               std::string name):
        id(id), x(x), y(y), dir(dir), skin(skin), name(std::move(name)) {}

void NewPlayerEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::NEW_PLAYER));
    proto.send_two_bytes_number(id);
    proto.send_two_bytes_number(static_cast<uint16_t>(x));
    proto.send_two_bytes_number(static_cast<uint16_t>(y));
    proto.sendByte(dir);
    proto.sendByte(skin);
    proto.send_two_bytes_number(static_cast<uint16_t>(name.size()));
    proto.send_message(std::vector<char>(name.begin(), name.end()));
}

std::unique_ptr<NewPlayerEvent> NewPlayerEvent::deserialize(CommonProtocol& proto) {
    uint16_t id = proto.receive_two_bytes_number();
    int16_t x = static_cast<int16_t>(proto.receive_two_bytes_number());
    int16_t y = static_cast<int16_t>(proto.receive_two_bytes_number());
    uint8_t dir = proto.receive_byte();
    uint8_t skin = proto.receive_byte();
    uint16_t nameLen = proto.receive_two_bytes_number();
    std::string name = proto.receive_message(nameLen);
    return std::make_unique<NewPlayerEvent>(id, x, y, dir, skin, std::move(name));
}

// ── PlayerMovedEvent ─────────────────────────────────────────────────────

PlayerMovedEvent::PlayerMovedEvent(uint16_t id, int16_t x, int16_t y, uint8_t dir):
        id(id), x(x), y(y), dir(dir) {}

void PlayerMovedEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::PLAYER_MOVED));
    proto.send_two_bytes_number(id);
    proto.send_two_bytes_number(static_cast<uint16_t>(x));
    proto.send_two_bytes_number(static_cast<uint16_t>(y));
    proto.sendByte(dir);
}

std::unique_ptr<PlayerMovedEvent> PlayerMovedEvent::deserialize(CommonProtocol& proto) {
    uint16_t id = proto.receive_two_bytes_number();
    int16_t x = static_cast<int16_t>(proto.receive_two_bytes_number());
    int16_t y = static_cast<int16_t>(proto.receive_two_bytes_number());
    uint8_t dir = proto.receive_byte();
    return std::make_unique<PlayerMovedEvent>(id, x, y, dir);
}

// ── PlayerDisconnectedEvent ──────────────────────────────────────────────

PlayerDisconnectedEvent::PlayerDisconnectedEvent(uint16_t id): id(id) {}

void PlayerDisconnectedEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::PLAYER_DISCONNECTED));
    proto.send_two_bytes_number(id);
}

std::unique_ptr<PlayerDisconnectedEvent> PlayerDisconnectedEvent::deserialize(
        CommonProtocol& proto) {
    uint16_t id = proto.receive_two_bytes_number();
    return std::make_unique<PlayerDisconnectedEvent>(id);
}

// ── StatsEvent ───────────────────────────────────────────────────────────

StatsEvent::StatsEvent(uint16_t hp, uint16_t maxHp, uint16_t mana, uint16_t maxMana, uint32_t gold,
                       uint32_t exp, uint32_t nextLvlExp, uint8_t level):
        hp(hp),
        maxHp(maxHp),
        mana(mana),
        maxMana(maxMana),
        gold(gold),
        exp(exp),
        nextLvlExp(nextLvlExp),
        level(level) {}

void StatsEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::STATS_JUGADOR));
    proto.send_two_bytes_number(hp);
    proto.send_two_bytes_number(maxHp);
    proto.send_two_bytes_number(mana);
    proto.send_two_bytes_number(maxMana);
    proto.send_four_bytes_number(gold);
    proto.send_four_bytes_number(exp);
    proto.send_four_bytes_number(nextLvlExp);
    proto.sendByte(level);
}

std::unique_ptr<StatsEvent> StatsEvent::deserialize(CommonProtocol& proto) {
    uint16_t hp = proto.receive_two_bytes_number();
    uint16_t maxHp = proto.receive_two_bytes_number();
    uint16_t mana = proto.receive_two_bytes_number();
    uint16_t maxMana = proto.receive_two_bytes_number();
    uint32_t gold = proto.receive_four_bytes_number();
    uint32_t exp = proto.receive_four_bytes_number();
    uint32_t nextLvlExp = proto.receive_four_bytes_number();
    uint8_t level = proto.receive_byte();
    return std::make_unique<StatsEvent>(hp, maxHp, mana, maxMana, gold, exp, nextLvlExp, level);
}

// ── AttackResultEvent ────────────────────────────────────────────────────

AttackResultEvent::AttackResultEvent(uint16_t attackerId, uint8_t targetType, uint16_t targetId,
                                     uint16_t damage, bool hit):
        attackerId(attackerId),
        targetType(targetType),
        targetId(targetId),
        damage(damage),
        hit(hit) {}

void AttackResultEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::ATTACK_RESULT));
    proto.send_two_bytes_number(attackerId);
    proto.sendByte(targetType);
    proto.send_two_bytes_number(targetId);
    proto.send_two_bytes_number(damage);
    proto.sendByte(hit ? 1 : 0);
}

std::unique_ptr<AttackResultEvent> AttackResultEvent::deserialize(CommonProtocol& proto) {
    uint16_t atk = proto.receive_two_bytes_number();
    uint8_t ttype = proto.receive_byte();
    uint16_t tid = proto.receive_two_bytes_number();
    uint16_t dmg = proto.receive_two_bytes_number();
    bool hit = (proto.receive_byte() != 0);
    return std::make_unique<AttackResultEvent>(atk, ttype, tid, dmg, hit);
}

// ── InventoryUpdateEvent ─────────────────────────────────────────────────

InventoryUpdateEvent::InventoryUpdateEvent(std::vector<uint8_t> items, uint8_t equippedWeapon,
                                           uint8_t equippedArmor, uint8_t equippedHelmet,
                                           uint8_t equippedShield):
        items(std::move(items)),
        equippedWeapon(equippedWeapon),
        equippedArmor(equippedArmor),
        equippedHelmet(equippedHelmet),
        equippedShield(equippedShield) {}

void InventoryUpdateEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::INVENTORY_UPDATE));
    proto.sendByte(static_cast<uint8_t>(items.size()));
    for (uint8_t id: items) {
        proto.sendByte(id);
    }
    proto.sendByte(equippedWeapon);
    proto.sendByte(equippedArmor);
    proto.sendByte(equippedHelmet);
    proto.sendByte(equippedShield);
}

std::unique_ptr<InventoryUpdateEvent> InventoryUpdateEvent::deserialize(CommonProtocol& proto) {
    uint8_t count = proto.receive_byte();
    std::vector<uint8_t> items;
    items.reserve(count);
    for (uint8_t i = 0; i < count; i++) {
        items.push_back(proto.receive_byte());
    }
    uint8_t eqW = proto.receive_byte();
    uint8_t eqA = proto.receive_byte();
    uint8_t eqH = proto.receive_byte();
    uint8_t eqS = proto.receive_byte();
    return std::make_unique<InventoryUpdateEvent>(std::move(items), eqW, eqA, eqH, eqS);
}

// ── PlayerEquippedEvent ──────────────────────────────────────────────────

PlayerEquippedEvent::PlayerEquippedEvent(uint16_t playerId, uint8_t slot, uint8_t itemId):
        playerId(playerId), slot(slot), itemId(itemId) {}

void PlayerEquippedEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::PLAYER_EQUIPPED));
    proto.send_two_bytes_number(playerId);
    proto.sendByte(slot);
    proto.sendByte(itemId);
}

std::unique_ptr<PlayerEquippedEvent> PlayerEquippedEvent::deserialize(CommonProtocol& proto) {
    uint16_t pid = proto.receive_two_bytes_number();
    uint8_t slot = proto.receive_byte();
    uint8_t itemId = proto.receive_byte();
    return std::make_unique<PlayerEquippedEvent>(pid, slot, itemId);
}

// ── NewNpcEvent ──────────────────────────────────────────────────────────

NewNpcEvent::NewNpcEvent(uint16_t id, int16_t x, int16_t y, uint8_t type, bool alive):
        id(id), x(x), y(y), type(type), alive(alive) {}

void NewNpcEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::NEW_NPC));
    proto.send_two_bytes_number(id);
    proto.send_two_bytes_number(static_cast<uint16_t>(x));
    proto.send_two_bytes_number(static_cast<uint16_t>(y));
    proto.sendByte(type);
    proto.sendByte(alive ? 1 : 0);
}

std::unique_ptr<NewNpcEvent> NewNpcEvent::deserialize(CommonProtocol& proto) {
    uint16_t id = proto.receive_two_bytes_number();
    int16_t x = static_cast<int16_t>(proto.receive_two_bytes_number());
    int16_t y = static_cast<int16_t>(proto.receive_two_bytes_number());
    uint8_t type = proto.receive_byte();
    bool alive = (proto.receive_byte() != 0);
    return std::make_unique<NewNpcEvent>(id, x, y, type, alive);
}

// ── NpcMovedEvent ────────────────────────────────────────────────────────

NpcMovedEvent::NpcMovedEvent(uint16_t id, int16_t x, int16_t y, uint8_t dir):
        id(id), x(x), y(y), dir(dir) {}

void NpcMovedEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::NPC_MOVED));
    proto.send_two_bytes_number(id);
    proto.send_two_bytes_number(static_cast<uint16_t>(x));
    proto.send_two_bytes_number(static_cast<uint16_t>(y));
    proto.sendByte(dir);
}

std::unique_ptr<NpcMovedEvent> NpcMovedEvent::deserialize(CommonProtocol& proto) {
    uint16_t id = proto.receive_two_bytes_number();
    int16_t x = static_cast<int16_t>(proto.receive_two_bytes_number());
    int16_t y = static_cast<int16_t>(proto.receive_two_bytes_number());
    uint8_t dir = proto.receive_byte();
    return std::make_unique<NpcMovedEvent>(id, x, y, dir);
}

// ── NpcDiedEvent ─────────────────────────────────────────────────────────

NpcDiedEvent::NpcDiedEvent(uint16_t id): id(id) {}

void NpcDiedEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::NPC_DIED));
    proto.send_two_bytes_number(id);
}

std::unique_ptr<NpcDiedEvent> NpcDiedEvent::deserialize(CommonProtocol& proto) {
    uint16_t id = proto.receive_two_bytes_number();
    return std::make_unique<NpcDiedEvent>(id);
}

// ── NpcRespawnedEvent ────────────────────────────────────────────────────

NpcRespawnedEvent::NpcRespawnedEvent(uint16_t id, int16_t x, int16_t y): id(id), x(x), y(y) {}

void NpcRespawnedEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ServerMsg::NPC_RESPAWNED));
    proto.send_two_bytes_number(id);
    proto.send_two_bytes_number(static_cast<uint16_t>(x));
    proto.send_two_bytes_number(static_cast<uint16_t>(y));
}

std::unique_ptr<NpcRespawnedEvent> NpcRespawnedEvent::deserialize(CommonProtocol& proto) {
    uint16_t id = proto.receive_two_bytes_number();
    int16_t x = static_cast<int16_t>(proto.receive_two_bytes_number());
    int16_t y = static_cast<int16_t>(proto.receive_two_bytes_number());
    return std::make_unique<NpcRespawnedEvent>(id, x, y);
}
