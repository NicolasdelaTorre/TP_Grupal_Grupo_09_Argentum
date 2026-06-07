#include "client_sender.h"

ClientSender::ClientSender(ClientProtocol& protocol, OutgoingQueue& clientEvents):
        protocol(protocol), clientEvents(clientEvents) {}

void ClientSender::run() {
    try {
        while (true) {
            auto ev = clientEvents.pop();
            protocol.send(*ev);
        }
    } catch (const ClosedQueue&) {
    }
}
