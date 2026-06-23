#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../common/Communication/common_protocol.h"

#include "protocol_loopback.h"

TEST(CommonProtocolTest, byteRoundTrip) {
    ProtocolLoopback link;
    link.a->sendByte(0xAB);
    EXPECT_EQ(link.b->receive_byte(), 0xAB);
}

TEST(CommonProtocolTest, twoBytesRoundTripRespectsNetworkOrder) {
    ProtocolLoopback link;
    link.a->send_two_bytes_number(0x1234);
    EXPECT_EQ(link.b->receive_two_bytes_number(), 0x1234);
}

TEST(CommonProtocolTest, fourBytesRoundTripRespectsNetworkOrder) {
    ProtocolLoopback link;
    link.a->send_four_bytes_number(0xDEADBEEF);
    EXPECT_EQ(link.b->receive_four_bytes_number(), 0xDEADBEEF);
}

TEST(CommonProtocolTest, twoBytesMaxValue) {
    ProtocolLoopback link;
    link.a->send_two_bytes_number(0xFFFF);
    EXPECT_EQ(link.b->receive_two_bytes_number(), 0xFFFF);
}

TEST(CommonProtocolTest, twoBytesZero) {
    ProtocolLoopback link;
    link.a->send_two_bytes_number(0);
    EXPECT_EQ(link.b->receive_two_bytes_number(), 0);
}

TEST(CommonProtocolTest, messageRoundTrip) {
    ProtocolLoopback link;
    std::string s = "hola mundo";
    link.a->send_message(std::vector<char>(s.begin(), s.end()));
    EXPECT_EQ(link.b->receive_message(s.size()), s);
}

TEST(CommonProtocolTest, messageWithSpecialCharacters) {
    ProtocolLoopback link;
    std::string s = "\x00ho\xFFla\x01";
    link.a->send_message(std::vector<char>(s.begin(), s.end()));
    EXPECT_EQ(link.b->receive_message(s.size()), s);
}

TEST(CommonProtocolTest, mixedFieldsSequence) {
    ProtocolLoopback link;
    link.a->sendByte(0x42);
    link.a->send_two_bytes_number(1024);
    link.a->send_four_bytes_number(70000);
    std::string name = "Tomas";
    link.a->send_two_bytes_number(static_cast<uint16_t>(name.size()));
    link.a->send_message(std::vector<char>(name.begin(), name.end()));

    EXPECT_EQ(link.b->receive_byte(), 0x42);
    EXPECT_EQ(link.b->receive_two_bytes_number(), 1024);
    EXPECT_EQ(link.b->receive_four_bytes_number(), 70000u);
    uint16_t len = link.b->receive_two_bytes_number();
    EXPECT_EQ(len, name.size());
    EXPECT_EQ(link.b->receive_message(len), name);
}
