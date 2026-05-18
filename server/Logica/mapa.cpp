#include "mapa.h"

Mapa::Mapa(uint16_t ancho, uint16_t alto): ancho(ancho), alto(alto) {
    celdas.resize(ancho * alto);
    inicializarMapa();
}

void Mapa::inicializarMapa() {
    for (auto& celda : celdas) {
        celda.esTransitable = true;
        celda.ocupadoPorJugador = false;
    }
}
