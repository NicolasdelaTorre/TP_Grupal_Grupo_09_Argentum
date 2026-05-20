//
// Created by nicolas on 17/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
#include <string>
#include <vector>

#include "../common/common_protocol.h"
#include "../common/socket.h"
#include "../common/DTOs.h"

class client_protocol {
    common_protocol protocol;

public:
    explicit client_protocol(Socket& skt);

    int send_username(const std::string& data);

    void send_message(const Command& command);

    ServerMessageType receive_message();
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
