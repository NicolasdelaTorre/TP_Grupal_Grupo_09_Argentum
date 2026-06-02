//
// Created by nicolas on 18/5/26.
//

#include "client_receiver.h"

client_receiver::client_receiver(client_protocol& protocol, Queue<ServerMessageType>& server_queue):
        protocol(protocol), server_queue(server_queue) {}

void client_receiver::run() {
    // LOGIN_OK + MAP se hacen sync en client::run(). Acá van solo los eventos post-login.
    try {
        while (should_keep_running()) {
            ServerMsg type = protocol.recv_msg_type();
            switch (type) {
                case ServerMsg::MOVE_OK:
                case ServerMsg::MOVE_FAIL:
                    // Confirmaciones — las ignoramos, el cliente predice localmente.
                    break;

                case ServerMsg::NEW_PLAYER: {
                    PlayerEvent ev = protocol.recv_new_player_payload();
                    server_queue.push("NEW_PLAYER:" + std::to_string(ev.id) + ":" +
                                      std::to_string(ev.x) + ":" + std::to_string(ev.y) + ":" +
                                      std::to_string(ev.dir) + ":" + ev.name);
                    break;
                }

                case ServerMsg::PLAYER_MOVED: {
                    PlayerEvent ev = protocol.recv_player_moved_payload();
                    server_queue.push("PLAYER_MOVED:" + std::to_string(ev.id) + ":" +
                                      std::to_string(ev.x) + ":" + std::to_string(ev.y) + ":" +
                                      std::to_string(ev.dir));
                    break;
                }

                case ServerMsg::PLAYER_DISCONNECTED: {
                    uint16_t id = protocol.recv_player_disconnected_payload();
                    server_queue.push("PLAYER_DISCONNECTED:" + std::to_string(id));
                    break;
                }

                case ServerMsg::STATS_JUGADOR: {
                    StatsEvent ev = protocol.recv_stats_payload();
                    server_queue.push("STATS:" + std::to_string(ev.health) + ":" +
                                      std::to_string(ev.maxHealth) + ":" +
                                      std::to_string(static_cast<int>(ev.level)));
                    break;
                }

                default:
                    break;
            }
        }
    } catch (const ClosedQueue&) {
    } catch (...) {
        // Socket cerrado o error de red — salimos limpio
    }
}
