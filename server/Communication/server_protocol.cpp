#include "server_protocol.h"

#include <utility>

#include <sys/socket.h>

#include "../../common/Communication/events/client_events.h"
#include "../../common/Communication/liberror.h"
#include "../../common/Communication/protocol_util.h"

ServerProtocol::ServerProtocol(const char* port):
        socketServer(port), clientSockets(), clientCounter(0) {}

int ServerProtocol::waitClient() {
    try {
        Socket clientSocket = socketServer.accept();
        clientCounter++;
        int clientId = clientCounter;
        clientSockets.emplace(clientId, CommonProtocol(std::move(clientSocket)));
        return clientId;
    } catch (const LibError& error) {
        if (ProtocolUtil::closedSocket(error))
            return 0;
        throw;
    }
}

void ServerProtocol::deleteClient(const int clientId) {
    auto it = clientSockets.find(clientId);
    if (it != clientSockets.end()) {
        it->second.shutdown();
        clientSockets.erase(it);
    }
}

std::shared_ptr<ClientEvent> ServerProtocol::receiveEvent(const int clientId) {
    auto it = clientSockets.find(clientId);
    if (it == clientSockets.end()) {
        return nullptr;
    }

    uint8_t opcode = it->second.receive_byte();
    if (!opcode) {
        return nullptr;
    }

    auto ev = ClientEvent::deserialize(opcode, it->second);
    ev->setPlayerId(clientId);
    return ev;
}

void ServerProtocol::send(int clientId, const ServerEvent& ev) {
    try {
        auto it = clientSockets.find(clientId);
        if (it == clientSockets.end())
            return;
        ev.serialize(it->second);
    } catch (const LibError& error) {
        if (!ProtocolUtil::closedSocket(error))
            throw;
    }
}

void ServerProtocol::disconnectServer() { socketServer.shutdown(SHUT_RDWR); }

ServerProtocol::~ServerProtocol() {
    for (auto& socket: clientSockets) {
        socket.second.shutdown();
    }
    socketServer.close();
}
