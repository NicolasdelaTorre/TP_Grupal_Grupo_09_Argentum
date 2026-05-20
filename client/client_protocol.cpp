//
// Created by nicolas on 17/5/26.
//

#include "client_protocol.h"

#include <cstdint>
#include <utility>

#include <netinet/in.h>

client_protocol::client_protocol(Socket skt): protocol(std::move(skt)) {}

int client_protocol::send(const std::string& data) {
    std::vector<char> buffer;
    buffer.push_back((uint8_t)0x01);
    buffer.push_back((uint8_t)htons(data.size()));
    buffer.push_back((uint8_t)data.size());
    buffer.insert(buffer.end(), data.begin(), data.end());
    return protocol.send(buffer.data(), buffer.size());
}
