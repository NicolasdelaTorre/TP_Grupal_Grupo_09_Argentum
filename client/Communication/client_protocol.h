#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_PROTOCOL_H

#include <cstdint>
#include <memory>

#include "../../common/Communication/common_protocol.h"
#include "../../common/Communication/events/client_event.h"
#include "../../common/Communication/events/server_event.h"
#include "../../common/Communication/socket.h"

class ClientProtocol {
private:
    CommonProtocol proto;

public:
    ClientProtocol(const char* hostname, const char* port);
    // Lee el próximo evento del server (opcode + payload)
    std::shared_ptr<ServerEvent> receiveEvent();
    // Manda un ClientEvent al server. El evento sabe serializarse a sí mismo.
    void send(const ClientEvent& ev);
    void shutdown();
};

#endif
