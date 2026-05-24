#include "client_handler.h"

ClientHandler::ClientHandler(ProtocolServer& protocol, Queue<std::string>& commands,
                             ClientMonitor& clientMonitor, const int clientId):
        clientQueue(),
        sender(protocol, clientQueue, clientId),
        receiver(protocol, commands, clientId),
        clientConnected(true),
        clientId(clientId) {
    clientMonitor.addQueues(clientId, clientQueue);
}

bool ClientHandler::clientDisconnected() { return !clientConnected; }

void ClientHandler::startThreads() {
    sender.start();
    receiver.start();
}

void ClientHandler::deleteClient() {
    sender.stop();
    receiver.stop();
}

ClientHandler::~ClientHandler() {
    sender.join();
    receiver.join();
}
