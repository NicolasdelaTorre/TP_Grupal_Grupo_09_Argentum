#ifndef SENDER_H
#define SENDER_H

#include <string>

#include "../common/queue.h"
#include "../common/thread.h"

#include "protocolo_server.h"

class Sender: public Thread {
private:
    ProtocoloServer& protocolo;
    Queue<std::string>& clienteMensajes;
    bool clienteConectado;
    const int idCliente;

public:
    Sender(ProtocoloServer& protocolo, Queue<std::string>& clienteMensajes, const int idCliente);

    virtual void run() override;

    virtual void stop() override;
};

#endif
