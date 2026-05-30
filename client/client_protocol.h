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

    // Envío del nombre de usuario al loguearse: [USER_ARRIVAL][len:2][name].
    int send_username(const std::vector<char>& data);

    // Las funciones recv_*_payload asumen que el opcode ya fue consumido vía recv_msg_type().
    Position recv_login_ok_payload();
    ReceivedMap recv_map();
    PlayerEvent recv_new_player_payload();
    PlayerEvent recv_player_moved_payload();
    uint16_t recv_player_disconnected_payload();

    // Envía la skin elegida en la pantalla de creación de personaje.
    void send_skin_selected(uint8_t skinId);
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
