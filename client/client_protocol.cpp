//
// Created by nicolas on 17/5/26.
//

#include "client_protocol.h"

#include <cstdint>
#include <utility>

#include <netinet/in.h>

client_protocol::client_protocol(Socket& skt): protocol(std::move(skt)) {}

int client_protocol::send_username(const std::string& data) {
    protocol.sendByte(0x01);
    protocol.send_message(data);
}

void client_protocol::send_message(const Command& command) {}
    
