#include "client_receiver.h"

ClientReceiver::ClientReceiver(ClientProtocol& protocol, IncomingQueue& serverEvents):
        protocol(protocol), serverEvents(serverEvents) {}

void ClientReceiver::run() {
    while (true) {
        auto ev = protocol.receiveEvent();
        if (!ev)
            break;
        serverEvents.push(std::move(ev));
    }
}
