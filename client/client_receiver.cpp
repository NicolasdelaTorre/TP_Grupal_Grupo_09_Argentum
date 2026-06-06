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
                                      std::to_string(ev.dir) + ":" + std::to_string(ev.skin) + ":" +
                                      ev.name);
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
                    server_queue.push(
                            "STATS:" + std::to_string(ev.health) + ":" +
                            std::to_string(ev.maxHealth) + ":" + std::to_string(ev.mana) + ":" +
                            std::to_string(ev.maxMana) + ":" + std::to_string(ev.gold) + ":" +
                            std::to_string(ev.experience) + ":" + std::to_string(ev.nextLevelExp) +
                            ":" + std::to_string(static_cast<int>(ev.level)));
                    break;
                }

                case ServerMsg::ATTACK_RESULT: {
                    AttackResultEvent ev = protocol.recv_attack_result_payload();
                    server_queue.push("ATTACK_RESULT:" + std::to_string(ev.attackerId) + ":" +
                                      std::to_string(static_cast<int>(ev.targetType)) + ":" +
                                      std::to_string(ev.targetId) + ":" +
                                      std::to_string(ev.damage) + ":" +
                                      std::to_string(ev.hit ? 1 : 0));
                    break;
                }

                case ServerMsg::PLAYER_EQUIPPED: {
                    EquipmentEvent ev = protocol.recv_player_equipped_payload();
                    server_queue.push("EQUIPPED:" + std::to_string(ev.playerId) + ":" +
                                      std::to_string(static_cast<int>(ev.slot)) + ":" +
                                      std::to_string(static_cast<int>(ev.itemId)));
                    break;
                }

                case ServerMsg::INVENTORY_UPDATE: {
                    InventoryEvent ev = protocol.recv_inventory_update_payload();
                    std::string msg = "INVENTORY:" + std::to_string(ev.items.size());
                    for (uint8_t id: ev.items) {
                        msg += ":" + std::to_string(static_cast<int>(id));
                    }
                    msg += ":" + std::to_string(static_cast<int>(ev.equippedWeapon));
                    msg += ":" + std::to_string(static_cast<int>(ev.equippedArmor));
                    msg += ":" + std::to_string(static_cast<int>(ev.equippedHelmet));
                    msg += ":" + std::to_string(static_cast<int>(ev.equippedShield));
                    server_queue.push(msg);
                    break;
                }

                case ServerMsg::DROPPED_ITEMS: {
                    auto items = protocol.recv_dropped_items_payload();
                    std::string msg = "DROPPED_ITEMS:" + std::to_string(items.size());
                    for (const auto& item: items) {
                        msg += ":" + std::to_string(item.x) + ":" + std::to_string(item.y) + ":" +
                               std::to_string(item.sheetId) + ":" + std::to_string(item.itemId);
                    }
                    server_queue.push(msg);
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
