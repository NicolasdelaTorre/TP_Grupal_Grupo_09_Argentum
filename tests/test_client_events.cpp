#include <memory>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "../common/Communication/events/client_event.h"
#include "../common/Communication/events/client_events.h"
#include "../common/Communication/message_types.h"

#include "protocol_loopback.h"

// Todos los tests siguen el mismo patron:
//   1. armar el evento original
//   2. serializarlo por link.a (lado cliente)
//   3. leer el opcode y deserializar por link.b (lado server)
//   4. dynamic_cast al tipo concreto
//   5. comparar los campos contra el original

TEST(ClientEventTest, userArrivalRoundTrip) {
    ProtocolLoopback link;

    UserArrivalEvent original("Tomas");
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<UserArrivalEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getName(), "Tomas");
}

TEST(ClientEventTest, userArrivalWithEmptyName) {
    ProtocolLoopback link;

    UserArrivalEvent original("");
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<UserArrivalEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getName(), "");
}

TEST(ClientEventTest, characterCreatedRoundTrip) {
    ProtocolLoopback link;

    CharacterCreatedEvent original(RaceCode::ELF, ClassCode::MAGE, 5, 2);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<CharacterCreatedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getRace(), RaceCode::ELF);
    EXPECT_EQ(recv->getClass(), ClassCode::MAGE);
    EXPECT_EQ(recv->getHeadId(), 5);
    EXPECT_EQ(recv->getSkinId(), 2);
}

TEST(ClientEventTest, movementRoundTripPerDirection) {
    for (auto dir: {MoveDirection::TOP, MoveDirection::BOTTOM, MoveDirection::LEFT,
                    MoveDirection::RIGHT}) {
        ProtocolLoopback link;

        MovementEvent original(dir);
        original.serialize(*link.a);

        uint8_t opcode = link.b->receive_byte();
        auto ev = ClientEvent::deserialize(opcode, *link.b);
        auto* recv = dynamic_cast<MovementEvent*>(ev.get());

        ASSERT_NE(recv, nullptr);
        EXPECT_EQ(recv->getDirection(), dir);
    }
}

TEST(ClientEventTest, turnRoundTrip) {
    ProtocolLoopback link;

    TurnEvent original(MoveDirection::LEFT);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<TurnEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getDirection(), MoveDirection::LEFT);
}

TEST(ClientEventTest, attackRoundTripTargetPlayer) {
    ProtocolLoopback link;

    AttackEvent original(0, 1234);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<AttackEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getTargetType(), 0);
    EXPECT_EQ(recv->getTargetId(), 1234);
}

TEST(ClientEventTest, attackRoundTripTargetNpc) {
    ProtocolLoopback link;

    AttackEvent original(1, 0xFFFF);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<AttackEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getTargetType(), 1);
    EXPECT_EQ(recv->getTargetId(), 0xFFFF);
}

TEST(ClientEventTest, pickUpItemRoundTrip) {
    ProtocolLoopback link;

    PickUpItemEvent original;
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<PickUpItemEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
}

TEST(ClientEventTest, dropItemRoundTrip) {
    ProtocolLoopback link;

    DropItemEvent original(3);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<DropItemEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getInvSlot(), 3);
}

TEST(ClientEventTest, equipItemRoundTrip) {
    ProtocolLoopback link;

    EquipItemEvent original(7);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<EquipItemEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getInvSlot(), 7);
}

TEST(ClientEventTest, unequipItemRoundTripPerSlot) {
    for (uint8_t slot = 0; slot < 4; ++slot) {
        ProtocolLoopback link;

        UnequipItemEvent original(slot);
        original.serialize(*link.a);

        uint8_t opcode = link.b->receive_byte();
        auto ev = ClientEvent::deserialize(opcode, *link.b);
        auto* recv = dynamic_cast<UnequipItemEvent*>(ev.get());

        ASSERT_NE(recv, nullptr);
        EXPECT_EQ(recv->getSlotType(), slot);
    }
}

TEST(ClientEventTest, selectNpcRoundTrip) {
    ProtocolLoopback link;

    SelectNpcEvent original(42);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<SelectNpcEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getNpcId(), 42);
}

TEST(ClientEventTest, chatMessageRoundTrip) {
    ProtocolLoopback link;

    ChatMessageEvent original("hola a todos");
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<ChatMessageEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getText(), "hola a todos");
}

TEST(ClientEventTest, chatMessageEmpty) {
    ProtocolLoopback link;

    ChatMessageEvent original("");
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<ChatMessageEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getText(), "");
}

TEST(ClientEventTest, chatMessageWithCommand) {
    ProtocolLoopback link;

    ChatMessageEvent original("/comprar 3");
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<ChatMessageEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getText(), "/comprar 3");
}

TEST(ClientEventTest, deserializeInvalidOpcodeThrows) {
    ProtocolLoopback link;
    EXPECT_THROW(ClientEvent::deserialize(0xEE, *link.b), std::runtime_error);
}
