#pragma once
#include <cstdint>

// Mensajes Cliente → Servidor
enum class ClientMsg : uint8_t {
    LLEGADA_USUARIO = 0x01,  // [opcode][len:2][nombre]
    MOVIMIENTO      = 0x02,  // [opcode][direccion:1]
    ARRIBA          = 0x03,
    ABAJO           = 0x04,
    IZQUIERDA       = 0x05,
    DERECHA         = 0x06,
};

// Mensajes Servidor → Cliente
enum class ServerMsg : uint8_t {
    LOGIN_OK           = 0x83,  // [opcode]
    LOGIN_FAIL         = 0x84,  // [opcode]
    POSICION_JUGADORES = 0x80,  // [opcode][cant:2][[id:1][x:2][y:2]...]
    STATS_JUGADOR      = 0x81,  // [opcode][vida:2][mana:2][exp:4][nivel:1][oro:4]
    CHAT_MSG           = 0x82,  // [opcode][len:2][texto]
};
