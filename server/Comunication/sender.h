#ifndef SENDER_H
#define SENDER_H

#include <string>

#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../Protocol/protocol_server.h"

class Sender: public Thread {
private:
    ProtocolServer& protocol;
    Queue<std::string>& clientMessages;
    bool clientConnected;
    const int clientId;

public:
    Sender(ProtocolServer& protocol, Queue<std::string>& clientMessages, const int clientId);

    virtual void run() override;

    virtual void stop() override;
};

#endif
