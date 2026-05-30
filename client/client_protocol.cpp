//
// Created by nicolas on 17/5/26.
//

#include "client_protocol.h"

#include <cstdint>
#include <stdexcept>
#include <utility>

#include <netinet/in.h>

client_protocol::client_protocol(Socket skt): protocol(std::move(skt)) {}

void client_protocol::send_move(ClientMsg direction) {
    protocol.sendByte(static_cast<uint8_t>(ClientMsg::MOVEMENT));
    protocol.sendByte(static_cast<uint8_t>(direction));
}

void client_protocol::send_turn(ClientMsg direction) {
    protocol.sendByte(static_cast<uint8_t>(ClientMsg::TURN));
    protocol.sendByte(static_cast<uint8_t>(direction));
}

ServerMsg client_protocol::recv_msg_type() {
    uint8_t opcode = protocol.receive_byte();
    return static_cast<ServerMsg>(opcode);
}

void client_protocol::close() { protocol.shutdown(); }


int client_protocol::send_username(const std::vector<char>& data) {
    protocol.sendByte(static_cast<uint8_t>(ClientMsg::USER_ARRIVAL));
    protocol.send_two_bytes_number(static_cast<uint16_t>(data.size()));
    protocol.send_message(data);
    return 0;
}

Position client_protocol::recv_login_ok_payload() {
    int16_t x = static_cast<int16_t>(protocol.receive_two_bytes_number());
    int16_t y = static_cast<int16_t>(protocol.receive_two_bytes_number());
    return Position{x, y};
}

ReceivedMap client_protocol::recv_map() {
    ReceivedMap result;
    result.width = protocol.receive_two_bytes_number();
    result.height = protocol.receive_two_bytes_number();
    uint16_t cellCount = protocol.receive_two_bytes_number();

    result.cells.reserve(cellCount);
    for (uint16_t i = 0; i < cellCount; i++) {
        ReceivedCell cell;
        cell.textureId = protocol.receive_two_bytes_number();
        cell.obstacleId = protocol.receive_two_bytes_number();
        cell.safeZone = (protocol.receive_byte() != 0);
        result.cells.push_back(cell);
    }
    return result;
}

PlayerEvent client_protocol::recv_new_player_payload() {
    PlayerEvent ev;
    ev.id = protocol.receive_two_bytes_number();
    ev.x = static_cast<int16_t>(protocol.receive_two_bytes_number());
    ev.y = static_cast<int16_t>(protocol.receive_two_bytes_number());
    ev.dir = protocol.receive_byte();
    uint16_t nameLen = protocol.receive_two_bytes_number();
    ev.name = protocol.receive_message(nameLen);
    return ev;
}

PlayerEvent client_protocol::recv_player_moved_payload() {
    PlayerEvent ev;
    ev.id = protocol.receive_two_bytes_number();
    ev.x = static_cast<int16_t>(protocol.receive_two_bytes_number());
    ev.y = static_cast<int16_t>(protocol.receive_two_bytes_number());
    ev.dir = protocol.receive_byte();
    return ev;
}

uint16_t client_protocol::recv_player_disconnected_payload() {
    return protocol.receive_two_bytes_number();
}

void client_protocol::send_skin_selected(uint8_t skinId) {
    protocol.sendByte(static_cast<uint8_t>(ClientMsg::SKIN_SELECTED));
    protocol.sendByte(skinId);
}
