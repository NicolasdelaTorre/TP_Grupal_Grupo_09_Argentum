#pragma once
#include <cstdint>

// Mensajes Cliente → Servidor
enum class ClientMsg : uint8_t {
    USER_ARRIVAL = 0x01,  // [opcode][len:2][nombre][len:2][raza][len:2][clase]
    MOVEMENT = 0x02,      // [opcode][direccion:1]
    TOP = 0x03,
    BOTTOM = 0x04,
    LEFT = 0x05,
    RIGHT = 0x06,
    SKIN_SELECTED = 0x07,  // [opcode][skin_id:1]
    TURN = 0x08,           // [opcode][direccion:1] — gira sin moverse de celda
    CHEAT = 0x09,          // [opcode][cheat_code:1] — cheat_code ∈ CheatCode (common/DTOs.h)
    ATTACK = 0x0A          // [opcode][direccion:1] — ataca al primero en línea de vista
};

// Mensajes Servidor → Cliente
enum class ServerMsg : uint8_t {
    POSICION_JUGADORES = 0x80,   // [opcode][cant:2][[id:1][x:2][y:2]...]
    STATS_JUGADOR = 0x81,        // [opcode][vida:2][maxVida:2][nivel:1]
                                 // (mana/exp/oro cuando estén implementados)
    CHAT_MSG = 0x82,             // [opcode][len:2][texto]
    MAP = 0x83,                  // [opcode][width:2][height:2][CellCount:2]
                                 // [[textureId:2][obstacleId:2][safeZone:1]]... (row-major)
    LOGIN_OK = 0x84,             // [opcode][spawn_x:2][spawn_y:2]
    LOGIN_FAIL = 0x85,           // [opcode]
    MOVE_OK = 0x86,              // [opcode]
    MOVE_FAIL = 0x87,            // [opcode]
    NEW_PLAYER = 0x88,           // [opcode][id:2][x:2][y:2][dir:1][skin:1][name_len:2][name:n]
    PLAYER_MOVED = 0x89,         // [opcode][id:2][x:2][y:2][dir:1]
    PLAYER_DISCONNECTED = 0x8A,  // [opcode][id:2]
    FIRST_LOGIN = 0x8B,          // [opcode] — usuario nuevo, debe crear personaje
    NPC_LIST = 0x8C,             // [opcode][count:2][[id:2][x:2][y:2][dir:1][type:1][moving:1]...]
    DROPPED_ITEMS = 0x8D,        // [opcode][count:2][[x:2][y:2][sheetId:1][itemId:2]...]
    ATTACK_RESULT = 0x8E         // [opcode][attacker_id:2][target_type:1][target_id:2][damage:2][hit:1]
                                 // target_type: 0=player, 1=npc. hit: 1=impactó, 0=evadió.
};
