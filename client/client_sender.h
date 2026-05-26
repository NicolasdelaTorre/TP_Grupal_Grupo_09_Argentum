//
// Created by nicolas on 18/5/26.
//

#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_SENDER_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_SENDER_H

#include <string>

#include "../common/DTOs.h"
#include "../common/queue.h"
#include "../common/thread.h"

#include "client_protocol.h"

class client_sender: public Thread {
private:
    client_protocol& protocol;
    // Queue<Command>& events_queue;
    Queue<std::string>& events_queue;

public:
    // client_sender(client_protocol& protocol, Queue<Command>& events_queue);
    client_sender(client_protocol& protocol, Queue<std::string>& events_queue);

    void run() override;

    bool is_alive() const override { return _is_alive; }
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_SENDER_H
