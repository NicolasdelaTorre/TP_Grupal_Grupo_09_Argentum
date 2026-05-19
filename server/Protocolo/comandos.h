#ifndef COMANDOS_H
#define COMANDOS_H

#include <cstdint>

enum class Comando : uint8_t {
    LLEGADA_USUARIO = 0x01,
    MOVIMIENTO = 0x02,
    ARRIBA = 0x03,
    ABAJO = 0x04,
    IZQUIERDA = 0x05,
    DERECHA = 0x06
};

#endif
