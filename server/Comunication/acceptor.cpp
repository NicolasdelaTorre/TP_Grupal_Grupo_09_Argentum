#include "acceptor.h"

Acceptor::Acceptor(ProtocolServer& protocol, Queue<std::string>& commands,
                   ClientMonitor& clientMonitor):
        protocol(protocol), clients(), commands(commands), clientMonitor(clientMonitor) {}

void Acceptor::run() {
    int clientId;
    while ((clientId = protocol.waitClient())) {
        auto* client = new ClientHandler(protocol, commands, clientMonitor, clientId);
        reap();
        clients.push_back(client);
        client->startThreads();
    }
    clear();
}

void Acceptor::reap() {
    clients.remove_if([this](const auto& client) {
        bool clientDisconnected = client->clientDisconnected();
        if (clientDisconnected) {
            protocol.deleteClient(client->clientId);
            clientMonitor.deleteQueue(client->clientId);
            delete client;
        }
        return clientDisconnected;
    });
}

void Acceptor::clear() {
    for (auto* client: clients) {
        client->deleteClient();
        protocol.deleteClient(client->clientId);
        clientMonitor.deleteQueue(client->clientId);
        delete client;
    }

    clients.clear();
}

void Acceptor::stop() {
    clear();
    protocol.disconnectServer();
}
