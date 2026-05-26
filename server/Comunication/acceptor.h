#ifndef ACEPTADOR_H
#define ACEPTADOR_H

#include <list>
#include <string>

#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../Protocol/protocol_server.h"

#include "client_handler.h"
#include "client_monitor.h"

class Acceptor: public Thread {
private:
    ProtocolServer& protocol;
    std::list<ClientHandler*> clients;
    Queue<std::string>& commands;
    ClientMonitor& clientMonitor;

    /*
     * Verify if a client has left the game. In that case, the clientHandler must be removed from the list of clients.
     */
    void reap();

    /*
     * Delete all clients from the list.
     */
    void clear();

public:
    Acceptor(ProtocolServer& protocol, Queue<std::string>& commands,
              ClientMonitor& clientMonitor);

    virtual void run() override;

    virtual void stop() override;
};

#endif
