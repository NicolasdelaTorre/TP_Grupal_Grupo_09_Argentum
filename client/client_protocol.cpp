//
// Created by nicolas on 17/5/26.
//

#include "client_protocol.h"

#include <cstdint>
#include <stdexcept>
#include <utility>

#include <netinet/in.h>

client_protocol::client_protocol(Socket& skt): protocol(std::move(skt)) {}

int client_protocol::send(const std::string& data) {
    uint8_t opcode = static_cast<uint8_t>(ClientMsg::LLEGADA_USUARIO);
    uint16_t len = htons(static_cast<uint16_t>(data.size()));
    protocol.sendByte(opcode);
    protocol.send_message(data);
    return 1;
}

void client_protocol::send_move(ClientMsg direction) {
    uint8_t buf[2] = {
        static_cast<uint8_t>(ClientMsg::MOVIMIENTO),
        static_cast<uint8_t>(direction)
    };
    protocol.sendByte(buf[0]);
    protocol.sendByte(buf[1]);
}

ServerMsg client_protocol::recv_msg_type() {
    uint8_t opcode = protocol.receive_byte();
    return static_cast<ServerMsg>(opcode);
}

void client_protocol::close() { protocol.shutdown(); }


int client_protocol::send_username(const std::string& data) {
    protocol.sendByte(0x01);
    protocol.send_message(data);
}

void client_protocol::send_message(const Command& command) {}
    
