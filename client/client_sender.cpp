//
// Created by nicolas on 18/5/26.
//

#include "client_sender.h"

// client_sender::client_sender(client_protocol& protocol, Queue<Command>& events_queue):
client_sender::client_sender(client_protocol& protocol, Queue<std::string>& events_queue):
        protocol(protocol), events_queue(events_queue) {}


// Formato de eventos esperados en la queue:
//   "TOP" / "BOTTOM" / "LEFT" / "RIGHT"             — cruzó un tile, send_move
//   "TURN_TOP" / "TURN_BOTTOM" / etc.               — giró sin moverse, send_turn
//   "ATTACK_TOP" / "ATTACK_BOTTOM" / etc.           — ataca en esa dirección
//   "CHEAT_SUICIDE" / "CHEAT_GOLD" / "CHEAT_EXPERIENCE" — sendCheat con el code
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
            else if (event == "CHEAT_SUICIDE")
                protocol.sendCheat(CheatCode::SUICIDE);
            else if (event == "CHEAT_GOLD")
                protocol.sendCheat(CheatCode::GOLD);
            else if (event == "CHEAT_EXPERIENCE")
                protocol.sendCheat(CheatCode::EXPERIENCE);
        }
    } catch (const ClosedQueue&) {}
}
