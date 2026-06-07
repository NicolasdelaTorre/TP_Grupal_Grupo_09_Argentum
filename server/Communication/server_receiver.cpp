#include "server_receiver.h"

#include "../../common/Communication/events/client_events.h"

ServerReceiver::ServerReceiver(ServerProtocol& protocol, IncomingQueue& clientEvents,
                               const int clientId):
        protocol(protocol), clientEvents(clientEvents), clientId(clientId) {}

void ServerReceiver::run() {
    while (true) {
        auto ev = protocol.receiveEvent(clientId);
        if (!ev)
            break;
        clientEvents.push(std::move(ev));
    }
    // El socket se cerró — encolamos un DisconnectEvent para que el Gameloop notifique a los demás y limpie el estado.
    auto disc = std::make_shared<DisconnectEvent>();
    disc->setPlayerId(clientId);
    clientEvents.push(disc);
}
