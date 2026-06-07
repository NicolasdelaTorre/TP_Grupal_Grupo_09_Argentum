#ifndef ACEPTADOR_H
#define ACEPTADOR_H

#include <list>

#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../Communication/server_protocol.h"

#include "client_handler.h"
#include "client_monitor.h"
#include "server_receiver.h"  // IncomingQueue alias

class Acceptor: public Thread {
private:
    ServerProtocol& protocol;
    std::list<ClientHandler*> clients;
    IncomingQueue& clientEvents;
    ClientMonitor& clientMonitor;

    /*
     * Verify if a client has left the game. In that case, the clientHandler must be removed from
     * the list of clients.
     */
    void reap();

    /*
     * Delete all clients from the list.
     */
    void clear();

public:
    Acceptor(ServerProtocol& protocol, IncomingQueue& clientEvents, ClientMonitor& clientMonitor);

    virtual void run() override;

    virtual void stop() override;
};

#endif
