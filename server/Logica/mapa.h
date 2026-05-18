#ifndef MAPA_H
#define MAPA_H

#include <cstdint>
#include <vector>

typedef struct Celda {
    bool esTransitable;
    bool ocupadoPorJugador;
} Celda;

class Mapa {
private:
    uint16_t ancho;
    uint16_t alto;
    std::vector<Celda> celdas;

public:
    Mapa(uint16_t ancho, uint16_t alto);

    // void agregarJugador();
};

#endif
