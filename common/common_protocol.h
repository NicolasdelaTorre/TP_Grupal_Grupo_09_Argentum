//
// Created by nicolas on 17/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_COMMON_PROTOCOL_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_COMMON_PROTOCOL_H

#include "socket.h"

class common_protocol
{
    Socket skt;

public:
    explicit common_protocol(Socket skt);
};


#endif //TP_GRUPAL_GRUPO_09_ARGENTUM_COMMON_PROTOCOL_H
