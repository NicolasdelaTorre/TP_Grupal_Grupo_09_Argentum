#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_RECEIVER_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_RECEIVER_H

#include <memory>

#include "../../common/Communication/events/server_event.h"
#include "../../common/queue.h"
#include "../../common/thread.h"

#include "client_protocol.h"

// Cola de entrada al cliente.
using IncomingQueue = Queue<std::shared_ptr<ServerEvent>>;

class ClientReceiver: public Thread {
private:
    ClientProtocol& protocol;
    IncomingQueue& serverEvents;

public:
    ClientReceiver(ClientProtocol& protocol, IncomingQueue& serverEvents);

    virtual void run() override;
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_RECEIVER_H
