#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_SENDER_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_SENDER_H

#include <memory>

#include "../../common/Communication/events/client_event.h"
#include "../../common/queue.h"
#include "../../common/thread.h"

#include "client_protocol.h"

// Cola de salida hacia el server.
using OutgoingQueue = Queue<std::shared_ptr<ClientEvent>>;

class ClientSender: public Thread {
private:
    ClientProtocol& protocol;
    OutgoingQueue& clientEvents;

public:
    ClientSender(ClientProtocol& protocol, OutgoingQueue& clientEvents);
    void run() override;
};


#endif  // TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_SENDER_H
