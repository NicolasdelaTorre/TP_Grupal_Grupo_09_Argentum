#ifndef COMANDOS_H
#define COMANDOS_H

#include <cstdint>

enum class Comando : uint8_t {
    LLEGADA_USUARIO = 0x01,
    ARRIBA = 0x02,
    ABAJO = 0x03,
    IZQUIERDA = 0x04,
    DERECHA = 0x05
};

#endif
