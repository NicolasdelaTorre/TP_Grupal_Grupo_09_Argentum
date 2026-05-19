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

    void cambiarPosicion(const std::string& direccion);

    int16_t getX();

    int16_t getY();
};

#endif
