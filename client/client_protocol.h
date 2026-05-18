//
// Created by nicolas on 17/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
#include "../common/socket.h"
#include "../common/common_protocol.h"

class client_protocol
{
    common_protocol protocol;

    public:
    explicit client_protocol(Socket skt);
};


#endif //TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
