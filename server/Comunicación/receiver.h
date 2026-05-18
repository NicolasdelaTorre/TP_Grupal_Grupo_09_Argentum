#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>

#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../Protocolo/protocolo_server.h"

class Receiver: public Thread {
private:
    ProtocoloServer& protocolo;
    Queue<std::string>& comandos;
    const int idCliente;

public:
    Receiver(ProtocoloServer& protocolo, Queue<std::string>& comandos, const int idCliente);

    virtual void run() override;
};

#endif
