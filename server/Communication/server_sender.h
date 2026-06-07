#ifndef SENDER_H
#define SENDER_H

#include <memory>

#include "../../common/Communication/events/server_event.h"
#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../Communication/server_protocol.h"

// Cola de salida hacia un cliente específico.
using OutgoingQueue = Queue<std::shared_ptr<ServerEvent>>;

class ServerSender: public Thread {
private:
    ServerProtocol& protocol;
    OutgoingQueue& serverEvents;
    const int clientId;

public:
    ServerSender(ServerProtocol& protocol, OutgoingQueue& serverEvents, const int clientId);
    virtual void run() override;
    virtual void stop() override;
};

#endif
