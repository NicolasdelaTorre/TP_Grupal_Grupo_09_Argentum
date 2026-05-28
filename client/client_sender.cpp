//
// Created by nicolas on 18/5/26.
//

#include "client_sender.h"

// client_sender::client_sender(client_protocol& protocol, Queue<Command>& events_queue):
client_sender::client_sender(client_protocol& protocol, Queue<std::string>& events_queue):
        protocol(protocol), events_queue(events_queue) {}


// Formato de eventos esperados en la queue: "TOP", "BOTTOM", "LEFT", "RIGHT"
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
        }
    } catch (const ClosedQueue&) {}
}
