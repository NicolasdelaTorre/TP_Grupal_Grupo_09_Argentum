#ifndef JUEGO_H
#define JUEGO_H

#include <string>
#include <unordered_map>

#include "jugador.h"
#include "mapa.h"

class Juego {
private:
    Mapa mapa;
    std::unordered_map<int, Jugador> jugadores;

    void procesarMovimiento(const int idJugador, const std::string& comando,
                            size_t pocisionComando);

public:
    Juego(uint16_t ancho, uint16_t alto);

    void procesarComando(const int idJugador, const std::string& comando);
};

#endif
