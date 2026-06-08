#include "server_sender.h"

ServerSender::ServerSender(ServerProtocol& protocol, OutgoingQueue& serverEvents,
                           const int clientId):
        protocol(protocol), serverEvents(serverEvents), clientId(clientId) {}


void ServerSender::run() {
    try {
        while (true) {
            auto ev = serverEvents.pop();
            protocol.send(clientId, *ev);
        }
    } catch (const ClosedQueue&) {
    }
}

void ServerSender::stop() { serverEvents.close(); }
