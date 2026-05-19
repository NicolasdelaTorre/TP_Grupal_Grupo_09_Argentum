#ifndef MAPA_H
#define MAPA_H

#include <cstdint>
#include <string>
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

    void inicializarMapa();

public:
    Mapa(uint16_t ancho, uint16_t alto);

    void agregarJugador();

    bool moverJugador(const std::string& direccion, int16_t x, int16_t y);
};

#endif
