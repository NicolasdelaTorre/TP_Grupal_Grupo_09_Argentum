#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_EVENT_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_CLIENT_EVENT_H

#include <cstdint>
#include <memory>

#include "../common_protocol.h"

// Clase base para todos los eventos que viajan cliente → server.
// Cada subclase encapsula sus datos + cómo serializarse al socket.
// Client side: construye la subclase, llama a serialize(socket).
// Server side: lee el opcode + llama a deserialize(opcode, socket) que devuelve la subclase, le setea el playerId, y la encola. El Gameloop pop la cola y hace dispatch por dynamic_cast (ver Gameloop::dispatch).
class ClientEvent {
protected:
    int playerId = 0;  // lo setea el Receiver del server después de deserializar

public:
    virtual ~ClientEvent() = default;
    virtual void serialize(CommonProtocol& proto) const = 0;
    static std::unique_ptr<ClientEvent> deserialize(uint8_t opcode, CommonProtocol& proto);
    int getPlayerId() const { return playerId; }
    void setPlayerId(int pid) { playerId = pid; }

};

#endif
