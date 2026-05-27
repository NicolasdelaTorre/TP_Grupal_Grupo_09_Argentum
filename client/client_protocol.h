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
#include "../common/socket.h"

class client_protocol {
    common_protocol protocol;

public:
    explicit client_protocol(Socket skt);

    // Enviar nombre de usuario al conectarse
    int send(const std::vector<char>& data);

    // Enviar movimiento: direction debe ser ARRIBA, ABAJO, IZQUIERDA o DERECHA
    void send_move(ClientMsg direction);

    // Leer el tipo del próximo mensaje que mandó el servidor
    ServerMsg recv_msg_type();

    // Cierra el socket — desbloquea cualquier recv pendiente
    void close();

    int send_username(const std::vector<char>& data);

    void send_message(const std::vector<char>& command);

    ServerMessageType receive_message();
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
