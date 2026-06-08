#include "client_monitor.h"

#include <utility>

ClientMonitor::ClientMonitor(): Clients(), mtx() {}

void ClientMonitor::addQueues(const int clientId, OutgoingQueue& serverEvents) {
    std::lock_guard<std::mutex> lock(mtx);
    Clients[clientId] = &serverEvents;
}

void ClientMonitor::deleteQueue(const int clientId) {
    std::lock_guard<std::mutex> lock(mtx);
    Clients.erase(clientId);
}

void ClientMonitor::broadcast(std::shared_ptr<ServerEvent> ev) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& cliente: Clients) {
        cliente.second->push(ev);
    }
}

void ClientMonitor::broadcastExcept(int excludeId, std::shared_ptr<ServerEvent> ev) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& cliente: Clients) {
        if (cliente.first == excludeId)
            continue;
        cliente.second->push(ev);
    }
}

void ClientMonitor::sendToClient(int clientId, std::shared_ptr<ServerEvent> ev) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = Clients.find(clientId);
    if (it != Clients.end()) {
        it->second->push(std::move(ev));
    }
}
