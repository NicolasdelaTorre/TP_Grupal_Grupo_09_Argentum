#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <atomic>

#include "../../common/queue.h"
#include "../Communication/server_protocol.h"

#include "client_monitor.h"
#include "server_receiver.h"
#include "server_sender.h"

class ClientHandler {
private:
    OutgoingQueue serverEvents;
    ServerSender sender;
    ServerReceiver receiver;
    std::atomic<bool> clientConnected;

public:
    const int clientId;

    /*
     * Create an object to receive messages from the client and another to send messages on behalf
     * of the server.
     */
    ClientHandler(ServerProtocol& protocol, IncomingQueue& clientEvents,
                  ClientMonitor& clientMonitor, const int clientId);

    bool clientDisconnected();

    void startThreads();

    /*
     * Force the closure of both threads.
     */
    void deleteClient();

    /*
     * Waits for the threads that send and receive messages from the client to finish.
     */
    ~ClientHandler();
};

#endif
