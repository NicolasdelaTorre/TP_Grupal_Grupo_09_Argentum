//
// Created by nicolas on 18/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_RECEIVER_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_RECEIVER_H
#include <string>

#include "../common/DTOs.h"
#include "../common/queue.h"
#include "../common/thread.h"

#include "client_protocol.h"

class client_receiver: public Thread {
private:
    client_protocol& protocol;
    Queue<ServerMessageType>& server_queue;

public:
    client_receiver(client_protocol& protocol, Queue<ServerMessageType>& server_queue);

    virtual void run() override;

    bool is_alive() const override { return _is_alive; }
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_RECEIVER_H
