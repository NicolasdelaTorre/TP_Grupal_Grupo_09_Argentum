//
// Created by nicolas on 18/5/26.
//

#include "client_sender.h"

client_sender::client_sender(client_protocol& protocol, Queue<std::string>& events_queue):
        protocol(protocol), events_queue(events_queue) {}

// Formato de eventos esperados en la queue: "ARRIBA", "ABAJO", "IZQUIERDA", "DERECHA"
void client_sender::run() {
    try {
        while (should_keep_running()) {
            std::string event = events_queue.pop();
            if (event == "ARRIBA")
                protocol.send_move(ClientMsg::ARRIBA);
            else if (event == "ABAJO")
                protocol.send_move(ClientMsg::ABAJO);
            else if (event == "IZQUIERDA")
                protocol.send_move(ClientMsg::IZQUIERDA);
            else if (event == "DERECHA")
                protocol.send_move(ClientMsg::DERECHA);
        }
    } catch (const ClosedQueue&) {}
}
