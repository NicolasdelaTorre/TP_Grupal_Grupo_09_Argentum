#pragma once
#include <cstdint>

// Mensajes Cliente → Servidor
enum class ClientMsg : uint8_t {
    USER_ARRIVAL  = 0x01,  // [opcode][len:2][nombre]
    MOVEMENT      = 0x02,  // [opcode][direccion:1]
    TOP           = 0x03,
    BOTTOM        = 0x04,
    LEFT          = 0x05,
    RIGHT         = 0x06,
    SKIN_SELECTED = 0x07   // [opcode][skin_id:1]
};

// Mensajes Servidor → Cliente
enum class ServerMsg : uint8_t {
    POSICION_JUGADORES = 0x80,  // [opcode][cant:2][[id:1][x:2][y:2]...]
    STATS_JUGADOR = 0x81,       // [opcode][vida:2][mana:2][exp:4][nivel:1][oro:4]
    CHAT_MSG = 0x82,            // [opcode][len:2][texto]
    MAP = 0x83,                 // [opcode][width:2][height:2][CellCount:2]
                                // [[textureId:2][obstacleId:2][safeZone:1]]... (row-major)
    LOGIN_OK = 0x84,            // [opcode][spawn_x:2][spawn_y:2]
    LOGIN_FAIL = 0x85,          // [opcode]
    MOVE_OK = 0x86,             // [opcode]
    MOVE_FAIL = 0x87,           // [opcode]
    NEW_PLAYER = 0x88,          // [opcode][id:2][x:2][y:2][name_len:2][name:n]
    PLAYER_MOVED = 0x89,        // [opcode][id:2][x:2][y:2]
    PLAYER_DISCONNECTED = 0x8A, // [opcode][id:2]
    FIRST_LOGIN         = 0x8B  // [opcode] — usuario nuevo, debe crear personaje
};
