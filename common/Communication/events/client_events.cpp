#include "client_events.h"

#include <stdexcept>
#include <utility>
#include <vector>

#include "../message_types.h"

// ── ClientEvent factory ──────────────────────────────────────────────────

std::unique_ptr<ClientEvent> ClientEvent::deserialize(uint8_t opcode, CommonProtocol& proto) {
    switch (static_cast<ClientMsg>(opcode)) {
        case ClientMsg::USER_ARRIVAL:
            return UserArrivalEvent::deserialize(proto);
        case ClientMsg::CHARACTER_CREATED:
            return CharacterCreatedEvent::deserialize(proto);
        case ClientMsg::MOVEMENT:
            return MovementEvent::deserialize(proto);
        case ClientMsg::TURN:
            return TurnEvent::deserialize(proto);
        case ClientMsg::ATTACK:
            return AttackEvent::deserialize(proto);
        case ClientMsg::PICK_UP_ITEM:
            return PickUpItemEvent::deserialize(proto);
        case ClientMsg::DROP_ITEM:
            return DropItemEvent::deserialize(proto);
        case ClientMsg::EQUIP_ITEM:
            return EquipItemEvent::deserialize(proto);
        case ClientMsg::UNEQUIP_ITEM:
            return UnequipItemEvent::deserialize(proto);
        case ClientMsg::CHAT:
            return ChatMessageEvent::deserialize(proto);
        case ClientMsg::SELECT_NPC:
            return SelectNpcEvent::deserialize(proto);
        default:
            throw std::runtime_error("ClientEvent: unknown opcode");
    }
}

// ── UserArrivalEvent ─────────────────────────────────────────────────────

UserArrivalEvent::UserArrivalEvent(std::string name): name(std::move(name)) {}

void UserArrivalEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::USER_ARRIVAL));
    proto.send_two_bytes_number(static_cast<uint16_t>(name.size()));
    proto.send_message(std::vector<char>(name.begin(), name.end()));
}

std::unique_ptr<UserArrivalEvent> UserArrivalEvent::deserialize(CommonProtocol& proto) {
    uint16_t nameLen = proto.receive_two_bytes_number();
    std::string name = nameLen ? proto.receive_message(nameLen) : "";
    return std::make_unique<UserArrivalEvent>(std::move(name));
}

// ── CharacterCreatedEvent ────────────────────────────────────────────────

CharacterCreatedEvent::CharacterCreatedEvent(RaceCode race, ClassCode class_, uint8_t headId,
                                             uint8_t skinId):
        race(race), class_(class_), headId(headId), skinId(skinId) {}

void CharacterCreatedEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::CHARACTER_CREATED));
    proto.sendByte(static_cast<uint8_t>(race));
    proto.sendByte(static_cast<uint8_t>(class_));
    proto.sendByte(headId);
    proto.sendByte(skinId);
}

std::unique_ptr<CharacterCreatedEvent> CharacterCreatedEvent::deserialize(CommonProtocol& proto) {
    auto race = static_cast<RaceCode>(proto.receive_byte());
    auto class_ = static_cast<ClassCode>(proto.receive_byte());
    uint8_t headId = proto.receive_byte();
    uint8_t skinId = proto.receive_byte();
    return std::make_unique<CharacterCreatedEvent>(race, class_, headId, skinId);
}

// ── MovementEvent ────────────────────────────────────────────────────────

MovementEvent::MovementEvent(MoveDirection direction): direction(direction) {}

void MovementEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::MOVEMENT));
    proto.sendByte(static_cast<uint8_t>(direction));
}

std::unique_ptr<MovementEvent> MovementEvent::deserialize(CommonProtocol& proto) {
    uint8_t dir = proto.receive_byte();
    return std::make_unique<MovementEvent>(static_cast<MoveDirection>(dir));
}

// ── TurnEvent ────────────────────────────────────────────────────────────

TurnEvent::TurnEvent(MoveDirection direction): direction(direction) {}

void TurnEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::TURN));
    proto.sendByte(static_cast<uint8_t>(direction));
}

std::unique_ptr<TurnEvent> TurnEvent::deserialize(CommonProtocol& proto) {
    uint8_t dir = proto.receive_byte();
    return std::make_unique<TurnEvent>(static_cast<MoveDirection>(dir));
}

// ── AttackEvent ──────────────────────────────────────────────────────────

AttackEvent::AttackEvent(uint8_t targetType, uint16_t targetId):
        targetType(targetType), targetId(targetId) {}

void AttackEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::ATTACK));
    proto.sendByte(targetType);
    proto.send_two_bytes_number(targetId);
}

std::unique_ptr<AttackEvent> AttackEvent::deserialize(CommonProtocol& proto) {
    uint8_t type = proto.receive_byte();
    uint16_t id = proto.receive_two_bytes_number();
    return std::make_unique<AttackEvent>(type, id);
}

// ── PickUpItemEvent ──────────────────────────────────────────────────────

void PickUpItemEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::PICK_UP_ITEM));
}

std::unique_ptr<PickUpItemEvent> PickUpItemEvent::deserialize(CommonProtocol&) {
    return std::make_unique<PickUpItemEvent>();
}

// ── DropItemEvent ────────────────────────────────────────────────────────

DropItemEvent::DropItemEvent(uint8_t invSlot): invSlot(invSlot) {}

void DropItemEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::DROP_ITEM));
    proto.sendByte(invSlot);
}

std::unique_ptr<DropItemEvent> DropItemEvent::deserialize(CommonProtocol& proto) {
    uint8_t slot = proto.receive_byte();
    return std::make_unique<DropItemEvent>(slot);
}

// ── EquipItemEvent ───────────────────────────────────────────────────────

EquipItemEvent::EquipItemEvent(uint8_t invSlot): invSlot(invSlot) {}

void EquipItemEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::EQUIP_ITEM));
    proto.sendByte(invSlot);
}

std::unique_ptr<EquipItemEvent> EquipItemEvent::deserialize(CommonProtocol& proto) {
    uint8_t slot = proto.receive_byte();
    return std::make_unique<EquipItemEvent>(slot);
}

// ── UnequipItemEvent ─────────────────────────────────────────────────────

UnequipItemEvent::UnequipItemEvent(uint8_t slotType): slotType(slotType) {}

void UnequipItemEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::UNEQUIP_ITEM));
    proto.sendByte(slotType);
}

std::unique_ptr<UnequipItemEvent> UnequipItemEvent::deserialize(CommonProtocol& proto) {
    uint8_t type = proto.receive_byte();
    return std::make_unique<UnequipItemEvent>(type);
}

// ── SelectNpcEvent ───────────────────────────────────────────────────────

SelectNpcEvent::SelectNpcEvent(uint16_t npcId): npcId(npcId) {}

void SelectNpcEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::SELECT_NPC));
    proto.send_two_bytes_number(npcId);
}

std::unique_ptr<SelectNpcEvent> SelectNpcEvent::deserialize(CommonProtocol& proto) {
    uint16_t id = proto.receive_two_bytes_number();
    return std::make_unique<SelectNpcEvent>(id);
}

// ── ChatMessageEvent ─────────────────────────────────────────────────────

ChatMessageEvent::ChatMessageEvent(std::string text): text(std::move(text)) {}

void ChatMessageEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::CHAT));
    proto.send_two_bytes_number(static_cast<uint16_t>(text.size()));
    proto.send_message(std::vector<char>(text.begin(), text.end()));
}

std::unique_ptr<ChatMessageEvent> ChatMessageEvent::deserialize(CommonProtocol& proto) {
    uint16_t len = proto.receive_two_bytes_number();
    std::string text = len ? proto.receive_message(len) : "";
    return std::make_unique<ChatMessageEvent>(std::move(text));
}

// ── DisconnectEvent ──────────────────────────────────────────────────────

void DisconnectEvent::serialize(CommonProtocol&) const {
    // no-op: este evento se construye server-side cuando se cierra el socket.
}
