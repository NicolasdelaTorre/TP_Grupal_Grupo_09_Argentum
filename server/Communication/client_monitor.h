#ifndef CLIENT_MONITOR_H
#define CLIENT_MONITOR_H

#include <map>
#include <memory>

#include "../../common/Communication/events/server_event.h"

#include "server_sender.h"  // OutgoingQueue

class ClientMonitor {
private:
    std::map<int, OutgoingQueue*> Clients;
    std::mutex mtx;

public:
    ClientMonitor();
    void addQueues(int clientId, OutgoingQueue& serverEvents);
    void deleteQueue(const int clientId);
    void broadcast(std::shared_ptr<ServerEvent> ev);
    void broadcastExcept(int excludeId, std::shared_ptr<ServerEvent> ev);
    void sendToClient(int clientId, std::shared_ptr<ServerEvent> ev);
};

#endif
