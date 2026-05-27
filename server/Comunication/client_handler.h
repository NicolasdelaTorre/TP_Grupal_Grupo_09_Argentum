#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <atomic>
#include <string>

#include "../../common/queue.h"
#include "../Protocol/protocol_server.h"

#include "client_monitor.h"
#include "receiver.h"
#include "sender.h"

class ClientHandler {
private:
    Queue<std::string> clientQueue;
    Sender sender;
    Receiver receiver;
    std::atomic<bool> clientConnected;

public:
    const int clientId;

    /*
     * Create an object to receive messages from the client and another to send messages on behalf
     * of the server.
     */
    ClientHandler(ProtocolServer& protocol, Queue<std::string>& commands,
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
