#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

#include <map>
#include <memory>

#include <netinet/in.h>

#include "../../common/Communication/common_protocol.h"
#include "../../common/Communication/events/client_event.h"
#include "../../common/Communication/events/server_event.h"
#include "../../common/Communication/liberror.h"
#include "../../common/Communication/socket.h"

class ServerProtocol {
private:
    Socket socketServer;
    std::map<int, CommonProtocol> clientSockets;
    int clientCounter;

public:
    explicit ServerProtocol(const char* port);
    int waitClient();
    void deleteClient(const int clientId);
    // Lee el próximo evento del server (opcode + payload)
    std::shared_ptr<ClientEvent> receiveEvent(const int clientId);
    // Manda un ServerEvent al cliente. El evento sabe serializarse a sí mismo.
    void send(int clientId, const ServerEvent& ev);
    void disconnectServer();
    ~ServerProtocol();
};

#endif
