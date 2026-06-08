#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_SERVER_EVENT_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_SERVER_EVENT_H

#include <cstdint>
#include <memory>

#include "../common_protocol.h"

// Clase base para todos los eventos que viajan server → cliente.
// Cada subclase encapsula sus datos + cómo serializarse al socket (server)
// y cómo deserializarse desde el socket (cliente).
// Server side: el Gameloop construye la subclase, la encola; el Sender thread la saca de la cola y llama a serialize(socket).
// Client side: el Receiver lee el opcode + llama a deserialize(opcode, socket)  que devuelve la subclase correcta, y la pasa al GameScreen.
class ServerEvent {
public:
    virtual ~ServerEvent() = default;
    virtual void serialize(CommonProtocol& proto) const = 0;
    static std::unique_ptr<ServerEvent> deserialize(uint8_t opcode, CommonProtocol& proto);
};

#endif
