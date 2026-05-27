//
// Created by nicolas on 17/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_COMMON_PROTOCOL_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_COMMON_PROTOCOL_H

#include <cstddef>
#include <string>
#include <vector>

#include <sys/types.h>

#include "socket.h"

class common_protocol {
    Socket skt;


public:
    explicit common_protocol(Socket skt);

    int sendByte(const u_int8_t byte);

    int send_two_bytes_number(u_int16_t number);

    u_int8_t receive_byte();

    u_int16_t receive_two_bytes_number();

    void send_message(std::vector<char> message);

    std::string receive_message(size_t size);

    void shutdown();
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_COMMON_PROTOCOL_H
