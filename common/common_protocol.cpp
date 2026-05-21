//
// Created by nicolas on 17/5/26.
//

#include "common_protocol.h"

#include <sys/socket.h>
#include <utility>
#include <arpa/inet.h>
#include <vector>


common_protocol::common_protocol(Socket skt): skt(std::move(skt)) {}

int common_protocol::send_two_bytes_number(const u_int16_t number) {
    const u_int16_t net_number = htons(number);
    return skt.sendall(&net_number, sizeof(net_number));
}

int common_protocol::sendByte(const u_int8_t byte) { return skt.sendall(&byte, sizeof(byte)); }

u_int8_t common_protocol::receive_byte() {
    u_int8_t byte;
    skt.recvall(&byte, sizeof(byte));
    return byte;
}

u_int16_t common_protocol::receive_two_bytes_number() {
    u_int16_t net_number;
    skt.recvall(&net_number, sizeof(net_number));
    return ntohs(net_number);
}

void common_protocol::send_message(const std::string& message) {
    const u_int16_t len = htons(message.size());
    skt.sendall(&len, sizeof(uint16_t));
    const std::vector<u_int8_t> data(message.begin(), message.end());
    skt.sendall(data.data(), data.size());
}

std::string common_protocol::receive_message(const size_t size) {
    std::vector<u_int8_t> data(size);
    skt.recvall(data.data(), data.size());
    return std::string(data.begin(), data.end());
}

void common_protocol::shutdown() {
    skt.shutdown(SHUT_RDWR);
    skt.close();
}
