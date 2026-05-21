//
// Created by nicolas on 17/5/26.
//

#include "client_protocol.h"

#include <cstdint>
#include <stdexcept>
#include <utility>

#include <netinet/in.h>

client_protocol::client_protocol(Socket skt): protocol(std::move(skt)) {}

int client_protocol::send(const std::string& data) {
    uint8_t opcode = static_cast<uint8_t>(ClientMsg::LLEGADA_USUARIO);
    uint16_t len = htons(static_cast<uint16_t>(data.size()));
    protocol.send(&opcode, 1);
    protocol.send(&len, 2);
    protocol.send(data.data(), data.size());
    return 1;
}

void client_protocol::send_move(ClientMsg direction) {
    uint8_t buf[2] = {
        static_cast<uint8_t>(ClientMsg::MOVIMIENTO),
        static_cast<uint8_t>(direction)
    };
    protocol.send(buf, 2);
}

ServerMsg client_protocol::recv_msg_type() {
    uint8_t opcode = 0;
    if (protocol.recv(&opcode, 1) == 0)
        throw std::runtime_error("connection closed");
    return static_cast<ServerMsg>(opcode);
}

void client_protocol::close() { protocol.close(); }
