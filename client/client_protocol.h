//
// Created by nicolas on 17/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
#include <string>
#include <vector>

#include "../common/DTOs.h"
#include "../common/common_protocol.h"
#include "../common/message_types.h"
#include "../common/position.h"
#include "../common/socket.h"

// Celda como la manda el servidor (walkable = obstacleId == 0).
struct ReceivedCell {
    uint16_t textureId;
    uint16_t obstacleId;
    bool safeZone;
};

// Mapa recibido del servidor. cells en row-major: idx = y * width + x.
struct ReceivedMap {
    uint16_t width;
    uint16_t height;
    std::vector<ReceivedCell> cells;
};

// Evento de un jugador (NEW_PLAYER y PLAYER_MOVED). En PLAYER_MOVED `name` queda vacío.
// dir usa valores wire: 3=TOP, 4=BOTTOM, 5=LEFT, 6=RIGHT.
struct PlayerEvent {
    uint16_t id;
    int16_t x;
    int16_t y;
    uint8_t dir;
    std::string name;
};

// Stats del jugador local (STATS_JUGADOR).
struct StatsEvent {
    uint16_t health;
    uint16_t maxHealth;
    uint8_t level;
};

class client_protocol {
    common_protocol protocol;

public:
    explicit client_protocol(Socket skt);

    // Enviar movimiento: direction debe ser TOP, BOTTOM, LEFT o RIGHT
    void send_move(ClientMsg direction);

    // Enviar cambio de dirección sin moverse (girar en la misma celda).
    void send_turn(ClientMsg direction);

    // Leer el tipo del próximo mensaje que mandó el servidor (1 byte de opcode)
    ServerMsg recv_msg_type();

    // Cierra el socket — desbloquea cualquier recv pendiente
    void close();

    // Envío del login: [USER_ARRIVAL][len:2][name][len:2][race][len:2][class].
    int send_user_arrival(const std::vector<char>& name, const std::string& race,
                          const std::string& class_);

    // Las funciones recv_*_payload asumen que el opcode ya fue consumido vía recv_msg_type().
    uint16_t recv_my_player_id();
    Position recv_login_ok_payload();
    ReceivedMap recv_map();
    PlayerEvent recv_new_player_payload();
    PlayerEvent recv_player_moved_payload();
    uint16_t recv_player_disconnected_payload();
    StatsEvent recv_stats_payload();

    // Envía la skin elegida en la pantalla de creación de personaje.
    void send_skin_selected(uint8_t skinId);

    // Recibe la lista completa de NPCs dinámicos (criaturas).
    std::vector<NpcEntity> recv_npc_list_payload();

    // Recibe la lista de items tirados en el piso.
    std::vector<DroppedItem> recv_dropped_items_payload();

    void sendCheat(CheatCode cheat);

    void send_attack(uint16_t attackerId, uint16_t targetId);
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
