#include "jugador.h"

Jugador::Jugador(const std::string& nombre): datos{nombre, {0, 0}} {}

void Jugador::cambiarPosicion(const std::string& direccion) {
    if (direccion == "arriba") {
        datos.posicion.y -= 1;
    } else if (direccion == "abajo") {
        datos.posicion.y += 1;
    } else if (direccion == "izquierda") {
        datos.posicion.x -= 1;
    } else if (direccion == "derecha") {
        datos.posicion.x += 1;
    }
}

int16_t Jugador::getX() { return datos.posicion.x; }

int16_t Jugador::getY() { return datos.posicion.y; }
