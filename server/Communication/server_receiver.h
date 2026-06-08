#ifndef RECEIVER_H
#define RECEIVER_H

#include <memory>

#include "../../common/Communication/events/client_event.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../Communication/server_protocol.h"

// Cola de entrada al server.
using IncomingQueue = Queue<std::shared_ptr<ClientEvent>>;

class ServerReceiver: public Thread {
private:
    ServerProtocol& protocol;
    IncomingQueue& clientEvents;
    const int clientId;

public:
    ServerReceiver(ServerProtocol& protocol, IncomingQueue& clientEvents, const int clientId);

    virtual void run() override;
};

#endif
