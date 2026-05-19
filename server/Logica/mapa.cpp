#include "mapa.h"

#include <algorithm>

Mapa::Mapa(uint16_t ancho, uint16_t alto): ancho(ancho), alto(alto) {
    celdas.resize(ancho * alto);
    inicializarMapa();
}

void Mapa::inicializarMapa() {
    for (auto& celda: celdas) {
        celda.esTransitable = true;
        celda.ocupadoPorJugador = false;
    }
}

void Mapa::agregarJugador() {
    for (;;) {
        auto itCeldaLibre = std::find_if(celdas.begin(), celdas.end(), [](const Celda& celda) {
            return celda.esTransitable && !celda.ocupadoPorJugador;
        });

        if (itCeldaLibre != celdas.end()) {
            itCeldaLibre->ocupadoPorJugador = true;
            return;
        }
    }
}

bool Mapa::moverJugador(const std::string& direccion, int16_t x, int16_t y) {
    int16_t nuevoX = x;
    int16_t nuevoY = y;

    if (direccion == "arriba") {
        nuevoY -= 1;
    } else if (direccion == "abajo") {
        nuevoY += 1;
    } else if (direccion == "izquierda") {
        nuevoX -= 1;
    } else if (direccion == "derecha") {
        nuevoX += 1;
    } else {
        // Dirección no válida
        return false;
    }

    if (nuevoX < 0 || nuevoX >= ancho || nuevoY < 0 || nuevoY >= alto) {
        // Fuera de los límites del mapa
        return false;
    }

    size_t nuevaPosicion = nuevoY * ancho + nuevoX;
    size_t posicionActual = y * ancho + x;

    if (!celdas[nuevaPosicion].esTransitable || celdas[nuevaPosicion].ocupadoPorJugador) {
        // Celda ocupada
        return false;
    }

    // Liberar la celda actual
    celdas[posicionActual].ocupadoPorJugador = false;
    // Ocupar la nueva celda
    celdas[nuevaPosicion].ocupadoPorJugador = true;

    // Movimiento exitoso
    return true;
}
