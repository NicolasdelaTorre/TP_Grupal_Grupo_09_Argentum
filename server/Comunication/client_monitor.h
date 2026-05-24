#ifndef CLIENT_MONITOR_H
#define CLIENT_MONITOR_H

#include <map>
#include <string>

#include "../../common/queue.h"

class ClientMonitor {
private:
    std::map<int, Queue<std::string>*> Clients;
    std::mutex mtx;

public:
    ClientMonitor();

    void addQueues(int clientId, Queue<std::string>& clientQueue);

    void deleteQueue(const int clientId);

    void broadcast(const std::string& message);

    // Send a message only to the client with that id. Does nothing if the id does not exist.
    void sendToClient(int clientId, const std::string& message);
};

#endif
