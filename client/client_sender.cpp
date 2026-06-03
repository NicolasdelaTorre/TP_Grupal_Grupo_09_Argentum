//
// Created by nicolas on 18/5/26.
//

#include "client_sender.h"

// client_sender::client_sender(client_protocol& protocol, Queue<Command>& events_queue):
client_sender::client_sender(client_protocol& protocol, Queue<std::string>& events_queue):
        protocol(protocol), events_queue(events_queue) {}


// Formato de eventos esperados en la queue:
//   "TOP" / "BOTTOM" / "LEFT" / "RIGHT"       — cruzó un tile, send_move
//   "TURN_TOP" / "TURN_BOTTOM" / etc.          — giró sin moverse, send_turn
//   "ATTACK:<attackerId>:<targetId>"            — ataque a otro jugador
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
            else if (event.rfind("ATTACK:", 0) == 0) {
                size_t c1 = event.find(':');
                size_t c2 = event.find(':', c1 + 1);
                uint16_t attackerId = static_cast<uint16_t>(std::stoi(event.substr(c1 + 1, c2 - c1 - 1)));
                uint16_t targetId   = static_cast<uint16_t>(std::stoi(event.substr(c2 + 1)));
                protocol.send_attack(attackerId, targetId);
            }
        }
    } catch (const ClosedQueue&) {}
}
