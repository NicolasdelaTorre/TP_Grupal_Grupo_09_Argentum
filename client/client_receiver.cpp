//
// Created by nicolas on 18/5/26.
//

#include "client_receiver.h"

client_receiver::client_receiver(client_protocol& protocol, Queue<ServerMessageType>& server_queue):
        protocol(protocol), server_queue(server_queue) {}

void client_receiver::run() {
    try {
        while (should_keep_running()) {
            ServerMsg type = protocol.recv_msg_type();
            switch (type) {
                case ServerMsg::LOGIN_OK:
                    server_queue.push("LOGIN_OK");
                    break;
                case ServerMsg::LOGIN_FAIL:
                    server_queue.push("LOGIN_FAIL");
                    break;
                default:
                    // Tipos aún no manejados — ignorar por ahora
                    break;
            }
        }
    } catch (const ClosedQueue&) {
    } catch (...) {
        // Socket cerrado o error de red — terminar el hilo limpiamente
    }
}
