#include "client_monitor.h"

ClientMonitor::ClientMonitor(): Clients(), mtx() {}

void ClientMonitor::addQueues(const int clientId, Queue<std::string>& clientQueue) {
    std::lock_guard<std::mutex> lock(mtx);
    Clients[clientId] = &clientQueue;
}

void ClientMonitor::deleteQueue(const int clientId) {
    std::lock_guard<std::mutex> lock(mtx);
    Clients.erase(clientId);
}

void ClientMonitor::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& cliente: Clients) {
        cliente.second->push(message);
    }
}

void ClientMonitor::broadcastExcept(int excludeId, const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& cliente: Clients) {
        if (cliente.first == excludeId)
            continue;
        cliente.second->push(message);
    }
}

void ClientMonitor::sendToClient(int clientId, const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = Clients.find(clientId);
    if (it != Clients.end()) {
        it->second->push(message);
    }
}
