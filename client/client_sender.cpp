//
// Created by nicolas on 18/5/26.
//

#include "client_sender.h"

// client_sender::client_sender(client_protocol& protocol, Queue<Command>& events_queue):
client_sender::client_sender(client_protocol& protocol, Queue<std::string>& events_queue):
        protocol(protocol), events_queue(events_queue) {}


// Formato de eventos esperados en la queue:
//   "TOP" / "BOTTOM" / "LEFT" / "RIGHT"                — cruzó un tile, send_move
//   "TURN_TOP" / "TURN_BOTTOM" / etc.                  — giró sin moverse, send_turn
//   "ATTACK_TOP" / "ATTACK_BOTTOM" / etc.              — ataca en esa dirección
//   "CHEAT_SUICIDE" / "CHEAT_GOLD" / "CHEAT_EXPERIENCE"— sendCheat con el code
//   "PICK_UP"                                          — /tomar
//   "DROP:<slot>" / "EQUIP:<slot>" / "UNEQUIP:<type>"  — inventario
void client_sender::run() {
    try {
        while (should_keep_running()) {
            std::string event = events_queue.pop();
            if (event == "TOP")
                protocol.send_move(ClientMsg::TOP);
            else if (event == "BOTTOM")
                protocol.send_move(ClientMsg::BOTTOM);
            else if (event == "LEFT")
                protocol.send_move(ClientMsg::LEFT);
            else if (event == "RIGHT")
                protocol.send_move(ClientMsg::RIGHT);
            else if (event == "TURN_TOP")
                protocol.send_turn(ClientMsg::TOP);
            else if (event == "TURN_BOTTOM")
                protocol.send_turn(ClientMsg::BOTTOM);
            else if (event == "TURN_LEFT")
                protocol.send_turn(ClientMsg::LEFT);
            else if (event == "TURN_RIGHT")
                protocol.send_turn(ClientMsg::RIGHT);
            else if (event == "ATTACK_TOP")
                protocol.send_attack(ClientMsg::TOP);
            else if (event == "ATTACK_BOTTOM")
                protocol.send_attack(ClientMsg::BOTTOM);
            else if (event == "ATTACK_LEFT")
                protocol.send_attack(ClientMsg::LEFT);
            else if (event == "ATTACK_RIGHT")
                protocol.send_attack(ClientMsg::RIGHT);
            else if (event.rfind("TARGETED_ATTACK:", 0) == 0) {
                // Formato: "TARGETED_ATTACK:<type>:<id>"
                size_t c1 = event.find(':');
                size_t c2 = event.find(':', c1 + 1);
                uint8_t type = static_cast<uint8_t>(std::stoi(event.substr(c1 + 1, c2 - c1 - 1)));
                uint16_t tid = static_cast<uint16_t>(std::stoi(event.substr(c2 + 1)));
                protocol.send_targeted_attack(type, tid);
            }
            else if (event == "CHEAT_SUICIDE")
                protocol.sendCheat(CheatCode::SUICIDE);
            else if (event == "CHEAT_GOLD")
                protocol.sendCheat(CheatCode::GOLD);
            else if (event == "CHEAT_EXPERIENCE")
                protocol.sendCheat(CheatCode::EXPERIENCE);
            else if (event == "PICK_UP")
                protocol.send_pick_up_item();
            else if (event.rfind("DROP:", 0) == 0) {
                uint8_t slot = static_cast<uint8_t>(std::stoi(event.substr(5)));
                protocol.send_drop_item(slot);
            }
            else if (event.rfind("EQUIP:", 0) == 0) {
                uint8_t slot = static_cast<uint8_t>(std::stoi(event.substr(6)));
                protocol.send_equip_item(slot);
            }
            else if (event.rfind("UNEQUIP:", 0) == 0) {
                uint8_t slotType = static_cast<uint8_t>(std::stoi(event.substr(8)));
                protocol.send_unequip_item(slotType);
            }
        }
    } catch (const ClosedQueue&) {}
}
