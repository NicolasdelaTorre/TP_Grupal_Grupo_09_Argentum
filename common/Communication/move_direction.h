#ifndef TP_GRUPAL_GRUPO_09_ARGENTUM_MOVE_DIRECTION_H
#define TP_GRUPAL_GRUPO_09_ARGENTUM_MOVE_DIRECTION_H

#include <cstdint>

// Direcciones de movimiento/giro que viajan en el wire (campos direction de
// MovementEvent y TurnEvent). Los valores coinciden con los usados antes en
// el enum ClientMsg (3=TOP, 4=BOTTOM, 5=LEFT, 6=RIGHT) para no romper el
// formato binario.
//
// Distinto de Direction (en common/DTOs.h), que es el índice de fila del
// spritesheet usado por el renderer del cliente.
enum class MoveDirection : uint8_t {
    TOP = 3,
    BOTTOM = 4,
    LEFT = 5,
    RIGHT = 6,
};

#endif
