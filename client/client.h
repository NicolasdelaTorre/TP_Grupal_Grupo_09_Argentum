//
// Created by nicolas on 16/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_H


#include <string>

#include "Communication/client_protocol.h"
#include "Communication/client_receiver.h"
#include "Communication/client_sender.h"


class Client {
    ClientProtocol protocol;
    OutgoingQueue clientEvents;
    IncomingQueue serverEvents;
    ClientSender sender;
    ClientReceiver receiver;
    bool fullscreen;

public:
    Client(const char* hostname, const char* port, bool fullscreen);

    void run();
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_H
