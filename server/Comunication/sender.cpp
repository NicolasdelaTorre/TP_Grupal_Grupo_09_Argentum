#include "sender.h"

Sender::Sender(ProtocolServer& protocol, Queue<std::string>& clientMessages,
               const int clientId):
        protocol(protocol),
        clientMessages(clientMessages),
        clientConnected(true),
        clientId(clientId) {}

void Sender::run() {
    std::string message;
    while (clientConnected) {
        try {
            message = clientMessages.pop();
        } catch (const ClosedQueue&) {
            clientConnected = false;
            break;
        }

        int sendState = protocol.sendMessage(message, clientId);

        if (sendState == 0) {
            // The client has disconnected
            break;
        }
    }
}

void Sender::stop() { clientMessages.close(); }
