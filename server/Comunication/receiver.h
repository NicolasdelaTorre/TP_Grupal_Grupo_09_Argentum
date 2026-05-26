#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>

#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../Protocol/protocol_server.h"

class Receiver: public Thread {
private:
    ProtocolServer& protocol;
    Queue<std::string>& commands;
    const int idCliente;

public:
    Receiver(ProtocolServer& protocol, Queue<std::string>& commands, const int idCliente);

    virtual void run() override;
};

#endif
