#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_EVENTS_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_EVENTS_H

#include <cstdint>
#include <memory>
#include <string>

#include "../../DTOs.h"

#include "client_event.h"

// USER_ARRIVAL: [opcode][name_len:2][name]. Solo el nombre: el server responde
// LOGIN_OK directo si el jugador ya existe, o FIRST_LOGIN si hay que crearlo.
class UserArrivalEvent: public ClientEvent {
private:
    std::string name;

public:
    explicit UserArrivalEvent(std::string name);
    const std::string& getName() const { return name; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<UserArrivalEvent> deserialize(CommonProtocol& proto);
};

// CHARACTER_CREATED: [opcode][race:1][class:1][headId:1][skinId:1]. Se manda
// solo despues de FIRST_LOGIN, juntando raza/clase/cabeza/skin de las pantallas
// de creacion.
class CharacterCreatedEvent: public ClientEvent {
private:
    RaceCode race;
    ClassCode class_;
    uint8_t headId;
    uint8_t skinId;

public:
    CharacterCreatedEvent(RaceCode race, ClassCode class_, uint8_t headId, uint8_t skinId);
    RaceCode getRace() const { return race; }
    ClassCode getClass() const { return class_; }
    uint8_t getHeadId() const { return headId; }
    uint8_t getSkinId() const { return skinId; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<CharacterCreatedEvent> deserialize(CommonProtocol& proto);
};

// MOVEMENT: [opcode][direction:1]. direction es un MoveDirection.
class MovementEvent: public ClientEvent {
private:
    MoveDirection direction;

public:
    explicit MovementEvent(MoveDirection direction);
    MoveDirection getDirection() const { return direction; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<MovementEvent> deserialize(CommonProtocol& proto);
};

// TURN: [opcode][direction:1]. Gira sin moverse.
class TurnEvent: public ClientEvent {
private:
    MoveDirection direction;

public:
    explicit TurnEvent(MoveDirection direction);
    MoveDirection getDirection() const { return direction; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<TurnEvent> deserialize(CommonProtocol& proto);
};

// ATTACK: [opcode][target_type:1][target_id:2]. Click sobre target.
class AttackEvent: public ClientEvent {
private:
    uint8_t targetType;  // 0=player, 1=npc
    uint16_t targetId;

public:
    AttackEvent(uint8_t targetType, uint16_t targetId);
    uint8_t getTargetType() const { return targetType; }
    uint16_t getTargetId() const { return targetId; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<AttackEvent> deserialize(CommonProtocol& proto);
};

// PICK_UP_ITEM: [opcode]. Server resuelve por posición.
class PickUpItemEvent: public ClientEvent {
public:
    PickUpItemEvent() = default;
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<PickUpItemEvent> deserialize(CommonProtocol& proto);
};

// DROP_ITEM: [opcode][inv_slot:1].
class DropItemEvent: public ClientEvent {
private:
    uint8_t invSlot;

public:
    explicit DropItemEvent(uint8_t invSlot);
    uint8_t getInvSlot() const { return invSlot; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<DropItemEvent> deserialize(CommonProtocol& proto);
};

// EQUIP_ITEM: [opcode][inv_slot:1]. Equipa, o usa si es poción.
class EquipItemEvent: public ClientEvent {
private:
    uint8_t invSlot;

public:
    explicit EquipItemEvent(uint8_t invSlot);
    uint8_t getInvSlot() const { return invSlot; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<EquipItemEvent> deserialize(CommonProtocol& proto);
};

// UNEQUIP_ITEM: [opcode][slot_type:1]. 0=arma, 1=armor, 2=casco, 3=escudo.
class UnequipItemEvent: public ClientEvent {
private:
    uint8_t slotType;

public:
    explicit UnequipItemEvent(uint8_t slotType);
    uint8_t getSlotType() const { return slotType; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<UnequipItemEvent> deserialize(CommonProtocol& proto);
};

// SELECT_NPC: [opcode][npc_id:2].
class SelectNpcEvent: public ClientEvent {
private:
    uint16_t npcId;

public:
    explicit SelectNpcEvent(uint16_t npcId);
    uint16_t getNpcId() const { return npcId; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<SelectNpcEvent> deserialize(CommonProtocol& proto);
};

// CHAT: [opcode][len:2][texto]. Texto libre. Si empieza con '/', el server lo trata como comando (cheat, comprar, etc.) y NO lo broadcastea.
class ChatMessageEvent: public ClientEvent {
private:
    std::string text;

public:
    explicit ChatMessageEvent(std::string text);
    const std::string& getText() const { return text; }
    void serialize(CommonProtocol& proto) const override;
    static std::unique_ptr<ChatMessageEvent> deserialize(CommonProtocol& proto);
};

// DisconnectEvent NO se serializa: el Receiver del server lo encola directamente cuando detecta que el socket del cliente se cerró
class DisconnectEvent: public ClientEvent {
public:
    DisconnectEvent() = default;
    void serialize(CommonProtocol& proto) const override;  // no-op
};

#endif
