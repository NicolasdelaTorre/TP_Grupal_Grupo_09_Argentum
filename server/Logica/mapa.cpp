#include "mapa.h"

Mapa::Mapa(uint16_t ancho, uint16_t alto): ancho(ancho), alto(alto) {
    celdas.resize(ancho * alto);
    // Deberia inicializar todas las celdas
}
