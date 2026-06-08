#include "client_handler.h"

ClientHandler::ClientHandler(ServerProtocol& protocol, IncomingQueue& clientEvents,
                             ClientMonitor& clientMonitor, const int clientId):
        serverEvents(),
        sender(protocol, serverEvents, clientId),
        receiver(protocol, clientEvents, clientId),
        clientConnected(true),
        clientId(clientId) {
    clientMonitor.addQueues(clientId, serverEvents);
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
