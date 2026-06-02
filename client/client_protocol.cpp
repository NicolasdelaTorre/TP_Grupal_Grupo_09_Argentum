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

void client_protocol::sendCheat(CheatCode cheat) {
    protocol.sendByte(static_cast<uint8_t>(cheat));
}

void client_protocol::close() { protocol.shutdown(); }


int client_protocol::send_user_arrival(const std::vector<char>& name, const std::string& race,
                                       const std::string& class_) {
    protocol.sendByte(static_cast<uint8_t>(ClientMsg::USER_ARRIVAL));
    protocol.send_two_bytes_number(static_cast<uint16_t>(name.size()));
    protocol.send_message(name);
    protocol.send_two_bytes_number(static_cast<uint16_t>(race.size()));
    protocol.send_message(std::vector<char>(race.begin(), race.end()));
    protocol.send_two_bytes_number(static_cast<uint16_t>(class_.size()));
    protocol.send_message(std::vector<char>(class_.begin(), class_.end()));
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

StatsEvent client_protocol::recv_stats_payload() {
    StatsEvent ev;
    ev.health = protocol.receive_two_bytes_number();
    ev.maxHealth = protocol.receive_two_bytes_number();
    ev.level = protocol.receive_byte();
    return ev;
}

void client_protocol::send_skin_selected(uint8_t skinId) {
    protocol.sendByte(static_cast<uint8_t>(ClientMsg::SKIN_SELECTED));
    protocol.sendByte(skinId);
}

std::vector<DroppedItem> client_protocol::recv_dropped_items_payload() {
    uint16_t count = protocol.receive_two_bytes_number();
    std::vector<DroppedItem> items;
    items.reserve(count);
    for (uint16_t i = 0; i < count; i++) {
        DroppedItem item;
        item.x = static_cast<int16_t>(protocol.receive_two_bytes_number());
        item.y = static_cast<int16_t>(protocol.receive_two_bytes_number());
        item.sheetId = protocol.receive_byte();
        item.itemId = protocol.receive_two_bytes_number();
        items.push_back(item);
    }
    return items;
}
