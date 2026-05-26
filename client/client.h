//
// Created by nicolas on 16/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_H


#include <string>

#include "../common/queue.h"

#include "client_protocol.h"
#include "client_receiver.h"
#include "client_sender.h"


class client {

    client_protocol protocol;
    std::string username;
    client_sender sender;
    client_receiver receiver;
    Queue<std::string> events_queue;
    Queue<std::string> server_queue;
    bool fullscreen;

public:
    client(const char* hostname, const char* port, bool fullscreen);

    void run();
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_H
