#ifndef JUGADOR_H
#define JUGADOR_H

#include <cstdint>
#include <string>

typedef struct Posicion {
    int16_t x;
    int16_t y;
} Posicion;

typedef struct DatosJugador {
    const std::string& nombre;
    Posicion posicion;
} DatosJugador;

class Jugador {
private:
    DatosJugador datos;

public:
    explicit Jugador(const std::string& nombre);
};

#endif
