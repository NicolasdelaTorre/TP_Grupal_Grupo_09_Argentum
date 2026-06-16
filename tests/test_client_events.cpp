#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "../common/Communication/events/client_event.h"
#include "../common/Communication/events/client_events.h"
#include "../common/Communication/message_types.h"

#include "protocol_loopback.h"

namespace {

template <typename T>
std::unique_ptr<T> roundTrip(const ClientEvent& src, ProtocolLoopback& link) {
    src.serialize(*link.a);
    uint8_t opcode = link.b->receive_byte();
    auto ev = ClientEvent::deserialize(opcode, *link.b);
    auto* casted = dynamic_cast<T*>(ev.get());
    if (!casted)
        return nullptr;
    ev.release();
    return std::unique_ptr<T>(casted);
}

}  // namespace

TEST(ClientEventTest, userArrivalRoundTrip) {
    ProtocolLoopback link;
    UserArrivalEvent original("Tomas");
    auto recv = roundTrip<UserArrivalEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getName(), "Tomas");
}

TEST(ClientEventTest, userArrivalConNombreVacio) {
    ProtocolLoopback link;
    UserArrivalEvent original("");
    auto recv = roundTrip<UserArrivalEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getName(), "");
}

TEST(ClientEventTest, characterCreatedRoundTrip) {
    ProtocolLoopback link;
    CharacterCreatedEvent original(RaceCode::ELF, ClassCode::MAGE, 5, 2);
    auto recv = roundTrip<CharacterCreatedEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getRace(), RaceCode::ELF);
    EXPECT_EQ(recv->getClass(), ClassCode::MAGE);
    EXPECT_EQ(recv->getHeadId(), 5);
    EXPECT_EQ(recv->getSkinId(), 2);
}

TEST(ClientEventTest, movementRoundTripPorCadaDireccion) {
    for (auto dir: {MoveDirection::TOP, MoveDirection::BOTTOM, MoveDirection::LEFT,
                    MoveDirection::RIGHT}) {
        ProtocolLoopback link;
        MovementEvent original(dir);
        auto recv = roundTrip<MovementEvent>(original, link);
        ASSERT_NE(recv, nullptr);
        EXPECT_EQ(recv->getDirection(), dir);
    }
}

TEST(ClientEventTest, turnRoundTrip) {
    ProtocolLoopback link;
    TurnEvent original(MoveDirection::LEFT);
    auto recv = roundTrip<TurnEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getDirection(), MoveDirection::LEFT);
}

TEST(ClientEventTest, attackRoundTripTargetPlayer) {
    ProtocolLoopback link;
    AttackEvent original(0, 1234);
    auto recv = roundTrip<AttackEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getTargetType(), 0);
    EXPECT_EQ(recv->getTargetId(), 1234);
}

TEST(ClientEventTest, attackRoundTripTargetNpc) {
    ProtocolLoopback link;
    AttackEvent original(1, 0xFFFF);
    auto recv = roundTrip<AttackEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getTargetType(), 1);
    EXPECT_EQ(recv->getTargetId(), 0xFFFF);
}

TEST(ClientEventTest, pickUpItemRoundTrip) {
    ProtocolLoopback link;
    PickUpItemEvent original;
    auto recv = roundTrip<PickUpItemEvent>(original, link);
    ASSERT_NE(recv, nullptr);
}

TEST(ClientEventTest, dropItemRoundTrip) {
    ProtocolLoopback link;
    DropItemEvent original(3);
    auto recv = roundTrip<DropItemEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getInvSlot(), 3);
}

TEST(ClientEventTest, equipItemRoundTrip) {
    ProtocolLoopback link;
    EquipItemEvent original(7);
    auto recv = roundTrip<EquipItemEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getInvSlot(), 7);
}

TEST(ClientEventTest, unequipItemRoundTripPorCadaSlot) {
    for (uint8_t slot = 0; slot < 4; ++slot) {
        ProtocolLoopback link;
        UnequipItemEvent original(slot);
        auto recv = roundTrip<UnequipItemEvent>(original, link);
        ASSERT_NE(recv, nullptr);
        EXPECT_EQ(recv->getSlotType(), slot);
    }
}

TEST(ClientEventTest, selectNpcRoundTrip) {
    ProtocolLoopback link;
    SelectNpcEvent original(42);
    auto recv = roundTrip<SelectNpcEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getNpcId(), 42);
}

TEST(ClientEventTest, chatMessageRoundTrip) {
    ProtocolLoopback link;
    ChatMessageEvent original("hola a todos");
    auto recv = roundTrip<ChatMessageEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getText(), "hola a todos");
}

TEST(ClientEventTest, chatMessageVacio) {
    ProtocolLoopback link;
    ChatMessageEvent original("");
    auto recv = roundTrip<ChatMessageEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getText(), "");
}

TEST(ClientEventTest, chatMessageConComandoCheat) {
    ProtocolLoopback link;
    ChatMessageEvent original("/comprar 3");
    auto recv = roundTrip<ChatMessageEvent>(original, link);
    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getText(), "/comprar 3");
}

TEST(ClientEventTest, deserializeOpcodeInvalidoTira) {
    ProtocolLoopback link;
    EXPECT_THROW(ClientEvent::deserialize(0xEE, *link.b), std::runtime_error);
}
