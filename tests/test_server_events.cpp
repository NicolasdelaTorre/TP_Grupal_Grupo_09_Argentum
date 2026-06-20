#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../common/Communication/events/server_event.h"
#include "../common/Communication/events/server_events.h"
#include "../common/Communication/message_types.h"

#include "protocol_loopback.h"

// Todos los tests siguen el mismo patron:
//   1. armar el evento original
//   2. serializarlo por link.a (lado server)
//   3. leer el opcode y deserializar por link.b (lado cliente)
//   4. dynamic_cast al tipo concreto
//   5. comparar los campos contra el original

TEST(ServerEventTest, loginFailOpcodeOnly) {
    ProtocolLoopback link;

    OpcodeOnlyEvent original(static_cast<uint8_t>(ServerMsg::LOGIN_FAIL));
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<OpcodeOnlyEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getOpcode(), static_cast<uint8_t>(ServerMsg::LOGIN_FAIL));
}

TEST(ServerEventTest, firstLoginOpcodeOnly) {
    ProtocolLoopback link;

    OpcodeOnlyEvent original(static_cast<uint8_t>(ServerMsg::FIRST_LOGIN));
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<OpcodeOnlyEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getOpcode(), static_cast<uint8_t>(ServerMsg::FIRST_LOGIN));
}

TEST(ServerEventTest, loginOkRoundTrip) {
    ProtocolLoopback link;

    LoginOkEvent original(100, 200, 1, 5);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<LoginOkEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getSpawnX(), 100);
    EXPECT_EQ(recv->getSpawnY(), 200);
    EXPECT_EQ(recv->getSkin(), 1);
    EXPECT_EQ(recv->getHead(), 5);
}

TEST(ServerEventTest, loginOkNegativeCoordinates) {
    ProtocolLoopback link;

    LoginOkEvent original(-1, -32768, 0, 0);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<LoginOkEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getSpawnX(), -1);
    EXPECT_EQ(recv->getSpawnY(), -32768);
}

TEST(ServerEventTest, mapEmptyRoundTrip) {
    ProtocolLoopback link;

    MapEvent original(10, 20, {});
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<MapEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getWidth(), 10);
    EXPECT_EQ(recv->getHeight(), 20);
    EXPECT_EQ(recv->getCells().size(), 0u);
    EXPECT_EQ(recv->getObstacles().size(), 0u);
}

TEST(ServerEventTest, mapWithCellsAndObstacles) {
    ProtocolLoopback link;

    std::vector<MapCellData> cells = {{1, 0, false}, {2, 5, true}, {3, 0, false}};
    std::vector<MapObstacleData> obstacles = {
        {-3, 4, 2, 1, "tree.png", 0},
        {10, 10, 3, 2, "walls/rock.png", 2}
    };
    MapEvent original(5, 5, cells, obstacles);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<MapEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    ASSERT_EQ(recv->getCells().size(), 3u);
    EXPECT_EQ(recv->getCells()[0].textureId, 1);
    EXPECT_EQ(recv->getCells()[0].obstacleId, 0);
    EXPECT_FALSE(recv->getCells()[0].safeZone);
    EXPECT_EQ(recv->getCells()[1].textureId, 2);
    EXPECT_EQ(recv->getCells()[1].obstacleId, 5);
    EXPECT_TRUE(recv->getCells()[1].safeZone);
    ASSERT_EQ(recv->getObstacles().size(), 2u);
    EXPECT_EQ(recv->getObstacles()[0].x, -3);
    EXPECT_EQ(recv->getObstacles()[0].y, 4);
    EXPECT_EQ(recv->getObstacles()[0].w, 2);
    EXPECT_EQ(recv->getObstacles()[0].h, 1);
    EXPECT_EQ(recv->getObstacles()[0].texture, "tree.png");
    EXPECT_EQ(recv->getObstacles()[0].texture_anchor, 0);
    EXPECT_EQ(recv->getObstacles()[1].texture, "walls/rock.png");
    EXPECT_EQ(recv->getObstacles()[1].texture_anchor, 2);
}

TEST(ServerEventTest, newPlayerRoundTrip) {
    ProtocolLoopback link;

    NewPlayerEvent original(7, 50, 60, 4, 1, 9, "Pepito");
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<NewPlayerEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getId(), 7);
    EXPECT_EQ(recv->getX(), 50);
    EXPECT_EQ(recv->getY(), 60);
    EXPECT_EQ(recv->getDir(), 4);
    EXPECT_EQ(recv->getSkin(), 1);
    EXPECT_EQ(recv->getHead(), 9);
    EXPECT_EQ(recv->getName(), "Pepito");
}

TEST(ServerEventTest, moveRejectedRoundTrip) {
    ProtocolLoopback link;

    MoveRejectedEvent original(15, 30);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<MoveRejectedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getX(), 15);
    EXPECT_EQ(recv->getY(), 30);
}

TEST(ServerEventTest, playerMovedRoundTrip) {
    ProtocolLoopback link;

    PlayerMovedEvent original(3, 25, 40, 6);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<PlayerMovedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getId(), 3);
    EXPECT_EQ(recv->getX(), 25);
    EXPECT_EQ(recv->getY(), 40);
    EXPECT_EQ(recv->getDir(), 6);
}

TEST(ServerEventTest, playerDisconnectedRoundTrip) {
    ProtocolLoopback link;

    PlayerDisconnectedEvent original(0xBEEF);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<PlayerDisconnectedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getId(), 0xBEEF);
}

TEST(ServerEventTest, statsRoundTrip) {
    ProtocolLoopback link;

    StatsEvent original(95, 100, 30, 50, 12345, 67890, 100000, 7);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<StatsEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getHp(), 95);
    EXPECT_EQ(recv->getMaxHp(), 100);
    EXPECT_EQ(recv->getMana(), 30);
    EXPECT_EQ(recv->getMaxMana(), 50);
    EXPECT_EQ(recv->getGold(), 12345u);
    EXPECT_EQ(recv->getExp(), 67890u);
    EXPECT_EQ(recv->getNextLvlExp(), 100000u);
    EXPECT_EQ(recv->getLevel(), 7);
}

TEST(ServerEventTest, statsGoldMax) {
    ProtocolLoopback link;

    StatsEvent original(0, 0, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<StatsEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getGold(), 0xFFFFFFFFu);
    EXPECT_EQ(recv->getExp(), 0xFFFFFFFFu);
    EXPECT_EQ(recv->getNextLvlExp(), 0xFFFFFFFFu);
    EXPECT_EQ(recv->getLevel(), 0xFF);
}

TEST(ServerEventTest, attackResultHit) {
    ProtocolLoopback link;

    AttackResultEvent original(1, 0, 5, 25, true);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<AttackResultEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getAttackerId(), 1);
    EXPECT_EQ(recv->getTargetType(), 0);
    EXPECT_EQ(recv->getTargetId(), 5);
    EXPECT_EQ(recv->getDamage(), 25);
    EXPECT_TRUE(recv->getHit());
}

TEST(ServerEventTest, attackResultEvade) {
    ProtocolLoopback link;

    AttackResultEvent original(2, 1, 99, 0, false);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<AttackResultEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getDamage(), 0);
    EXPECT_FALSE(recv->getHit());
}

TEST(ServerEventTest, inventoryUpdateRoundTrip) {
    ProtocolLoopback link;

    InventoryUpdateEvent original({10, 20, 30, 254}, 10, 20, 0xFF, 30);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<InventoryUpdateEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    ASSERT_EQ(recv->getItems().size(), 4u);
    EXPECT_EQ(recv->getItems()[0], 10);
    EXPECT_EQ(recv->getItems()[3], 254);
    EXPECT_EQ(recv->getEquippedWeapon(), 10);
    EXPECT_EQ(recv->getEquippedArmor(), 20);
    EXPECT_EQ(recv->getEquippedHelmet(), 0xFF);
    EXPECT_EQ(recv->getEquippedShield(), 30);
}

TEST(ServerEventTest, inventoryUpdateEmpty) {
    ProtocolLoopback link;

    InventoryUpdateEvent original({}, 0xFF, 0xFF, 0xFF, 0xFF);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<InventoryUpdateEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getItems().size(), 0u);
    EXPECT_EQ(recv->getEquippedWeapon(), 0xFF);
}

TEST(ServerEventTest, playerEquippedRoundTrip) {
    ProtocolLoopback link;

    PlayerEquippedEvent original(11, 2, 42);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<PlayerEquippedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getPlayerId(), 11);
    EXPECT_EQ(recv->getSlot(), 2);
    EXPECT_EQ(recv->getItemId(), 42);
}

TEST(ServerEventTest, newNpcAlive) {
    ProtocolLoopback link;

    NewNpcEvent original(50, 12, 34, 3, true);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<NewNpcEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_TRUE(recv->getAlive());
    EXPECT_EQ(recv->getType(), 3);
}

TEST(ServerEventTest, newNpcDead) {
    ProtocolLoopback link;

    NewNpcEvent original(51, 0, 0, 0, false);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<NewNpcEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_FALSE(recv->getAlive());
}

TEST(ServerEventTest, npcMovedRoundTrip) {
    ProtocolLoopback link;

    NpcMovedEvent original(60, 7, 8, 5);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<NpcMovedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getId(), 60);
    EXPECT_EQ(recv->getDir(), 5);
}

TEST(ServerEventTest, npcDiedRoundTrip) {
    ProtocolLoopback link;

    NpcDiedEvent original(77);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<NpcDiedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getId(), 77);
}

TEST(ServerEventTest, npcRespawnedRoundTrip) {
    ProtocolLoopback link;

    NpcRespawnedEvent original(88, 33, 44);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<NpcRespawnedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getX(), 33);
    EXPECT_EQ(recv->getY(), 44);
}

TEST(ServerEventTest, chatBroadcastFromPlayer) {
    ProtocolLoopback link;

    ChatBroadcastEvent original(5, "Tomas", "che, vamos al dungeon");
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<ChatBroadcastEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getAuthorId(), 5);
    EXPECT_EQ(recv->getAuthorName(), "Tomas");
    EXPECT_EQ(recv->getText(), "che, vamos al dungeon");
}

TEST(ServerEventTest, chatBroadcastFromSystem) {
    ProtocolLoopback link;

    ChatBroadcastEvent original(0, "", "Subiste a nivel 5");
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<ChatBroadcastEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getAuthorId(), 0);
    EXPECT_EQ(recv->getAuthorName(), "");
    EXPECT_EQ(recv->getText(), "Subiste a nivel 5");
}

TEST(ServerEventTest, playerDiedRoundTrip) {
    ProtocolLoopback link;

    PlayerDiedEvent original(13);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<PlayerDiedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getId(), 13);
}

TEST(ServerEventTest, playerRevivedRoundTrip) {
    ProtocolLoopback link;

    PlayerRevivedEvent original(13, 80, 90);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<PlayerRevivedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getId(), 13);
    EXPECT_EQ(recv->getX(), 80);
    EXPECT_EQ(recv->getY(), 90);
}

TEST(ServerEventTest, itemDroppedNormal) {
    ProtocolLoopback link;

    ItemDroppedEvent original(100, 5, 12, 34, 0);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<ItemDroppedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getDropId(), 100);
    EXPECT_EQ(recv->getItemId(), 5);
    EXPECT_EQ(recv->getX(), 12);
    EXPECT_EQ(recv->getY(), 34);
    EXPECT_EQ(recv->getGoldAmount(), 0u);
}

TEST(ServerEventTest, itemDroppedGold) {
    ProtocolLoopback link;

    ItemDroppedEvent original(101, 254, 5, 5, 9999);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<ItemDroppedEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getItemId(), 254);
    EXPECT_EQ(recv->getGoldAmount(), 9999u);
}

TEST(ServerEventTest, itemPickedUpRoundTrip) {
    ProtocolLoopback link;

    ItemPickedUpEvent original(200);
    original.serialize(*link.a);

    uint8_t opcode = link.b->receive_byte();
    auto ev = ServerEvent::deserialize(opcode, *link.b);
    auto* recv = dynamic_cast<ItemPickedUpEvent*>(ev.get());

    ASSERT_NE(recv, nullptr);
    EXPECT_EQ(recv->getDropId(), 200);
}

TEST(ServerEventTest, deserializeInvalidOpcodeThrows) {
    ProtocolLoopback link;
    EXPECT_THROW(ServerEvent::deserialize(0x00, *link.b), std::runtime_error);
}
