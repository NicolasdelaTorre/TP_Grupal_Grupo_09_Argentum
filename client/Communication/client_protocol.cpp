#include "client_protocol.h"

#include "../../common/Communication/events/client_events.h"
#include "../../common/Communication/events/server_events.h"

ClientProtocol::ClientProtocol(const char* hostname, const char* port):
        proto(Socket(hostname, port)) {}

std::shared_ptr<ServerEvent> ClientProtocol::receiveEvent() {
    uint8_t opcode = proto.receive_byte();
    if (!opcode)
        return nullptr;
    auto ev = ServerEvent::deserialize(opcode, proto);
    return std::shared_ptr<ServerEvent>(std::move(ev));
}

void ClientProtocol::send(const ClientEvent& ev) { ev.serialize(proto); }

void ClientProtocol::shutdown() { proto.shutdown(); }
