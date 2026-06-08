#pragma once
#include <cstdint>

// Mensajes Cliente → Servidor.
// Los valores del campo `direccion` de MOVEMENT/TURN viven en el enum
// MoveDirection (common/Communication/move_direction.h), no acá.
enum class ClientMsg : uint8_t {
    USER_ARRIVAL = 0x01,    // [opcode][len:2][nombre][len:2][raza][len:2][clase]
    MOVEMENT = 0x02,        // [opcode][direccion:1] — direccion ∈ MoveDirection
    SKIN_SELECTED = 0x07,   // [opcode][skin_id:1]
    TURN = 0x08,            // [opcode][direccion:1] — direccion ∈ MoveDirection
    ATTACK = 0x09,          // [opcode][target_type:1][target_id:2]
    HEAD_SELECTED = 0x0A,   // [opcode][head_id:1]
    PICK_UP_ITEM = 0x0D,    // [opcode]
    DROP_ITEM = 0x0E,       // [opcode][inv_slot:1]
    EQUIP_ITEM = 0x0F,      // [opcode][inv_slot:1] — equipa, o usa si es poción (consume)
    UNEQUIP_ITEM = 0x10,    // [opcode][slot_type:1] — 0=arma, 1=armadura, 2=casco, 3=escudo
    CHAT = 0x11             // [opcode][len:2][texto] — texto libre o /comando arg
};

// Mensajes Servidor → Cliente
enum class ServerMsg : uint8_t {
    POSICION_JUGADORES = 0x80,   // [opcode][cant:2][[id:1][x:2][y:2]...]
    STATS_JUGADOR = 0x81,        // [opcode][hp:2][maxHp:2][mana:2][maxMana:2][gold:4][exp:4][nextLevelExp:4][level:1]
    CHAT_MSG = 0x82,             // [opcode][author_id:2][name_len:2][name][msg_len:2][msg] — author_id=0 si es msg del sistema
    MAP = 0x83,                  // [opcode][width:2][height:2][CellCount:2][[textureId:2][obstacleId:2][safeZone:1]]...
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
    ATTACK_RESULT = 0x8E,  // [opcode][attacker_id:2][target_type:1][target_id:2][damage:2][hit:1] — target_type: 0=player, 1=npc. hit: 1=impactó, 0=evadió.
    INVENTORY_UPDATE = 0x8F,  // [opcode][count:1][[itemId:1]...×count][eqWeapon:1][eqArmor:1][eqHelmet:1][eqShield:1]
    PLAYER_EQUIPPED = 0x90,   // [opcode][playerId:2][slot:1][itemId:1]
    NEW_NPC = 0x91,           // [opcode][id:2][x:2][y:2][type:1][alive:1] — type = NpcType (spider=0, skeleton=1, ...). alive=0 si está muerto al snapshot.
    NPC_MOVED = 0x92,         // [opcode][id:2][x:2][y:2][dir:1]
    NPC_DIED = 0x93,          // [opcode][id:2]
    NPC_RESPAWNED = 0x94,     // [opcode][id:2][x:2][y:2]
};
