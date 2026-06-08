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
        case ClientMsg::MOVEMENT:
            return MovementEvent::deserialize(proto);
        case ClientMsg::TURN:
            return TurnEvent::deserialize(proto);
        case ClientMsg::SKIN_SELECTED:
            return SkinSelectedEvent::deserialize(proto);
        case ClientMsg::HEAD_SELECTED:
            return HeadSelectedEvent::deserialize(proto);
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
        default:
            throw std::runtime_error("ClientEvent: unknown opcode");
    }
}

// ── UserArrivalEvent ─────────────────────────────────────────────────────

UserArrivalEvent::UserArrivalEvent(std::string name, std::string race, std::string class_):
        name(std::move(name)), race(std::move(race)), class_(std::move(class_)) {}

void UserArrivalEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::USER_ARRIVAL));
    proto.send_two_bytes_number(static_cast<uint16_t>(name.size()));
    proto.send_message(std::vector<char>(name.begin(), name.end()));
    proto.send_two_bytes_number(static_cast<uint16_t>(race.size()));
    proto.send_message(std::vector<char>(race.begin(), race.end()));
    proto.send_two_bytes_number(static_cast<uint16_t>(class_.size()));
    proto.send_message(std::vector<char>(class_.begin(), class_.end()));
}

std::unique_ptr<UserArrivalEvent> UserArrivalEvent::deserialize(CommonProtocol& proto) {
    uint16_t nameLen = proto.receive_two_bytes_number();
    std::string name = nameLen ? proto.receive_message(nameLen) : "";
    uint16_t raceLen = proto.receive_two_bytes_number();
    std::string race = raceLen ? proto.receive_message(raceLen) : "";
    uint16_t classLen = proto.receive_two_bytes_number();
    std::string class_ = classLen ? proto.receive_message(classLen) : "";
    return std::make_unique<UserArrivalEvent>(std::move(name), std::move(race), std::move(class_));
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

// ── SkinSelectedEvent ────────────────────────────────────────────────────

SkinSelectedEvent::SkinSelectedEvent(uint8_t skinId): skinId(skinId) {}

void SkinSelectedEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::SKIN_SELECTED));
    proto.sendByte(skinId);
}

std::unique_ptr<SkinSelectedEvent> SkinSelectedEvent::deserialize(CommonProtocol& proto) {
    uint8_t id = proto.receive_byte();
    return std::make_unique<SkinSelectedEvent>(id);
}

// ── HeadSelectedEvent ────────────────────────────────────────────────────

HeadSelectedEvent::HeadSelectedEvent(uint8_t headId): headId(headId) {}

void HeadSelectedEvent::serialize(CommonProtocol& proto) const {
    proto.sendByte(static_cast<uint8_t>(ClientMsg::HEAD_SELECTED));
    proto.sendByte(headId);
}

std::unique_ptr<HeadSelectedEvent> HeadSelectedEvent::deserialize(CommonProtocol& proto) {
    uint8_t id = proto.receive_byte();
    return std::make_unique<HeadSelectedEvent>(id);
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
