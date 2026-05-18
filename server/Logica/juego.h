#ifndef JUEGO_H
#define JUEGO_H

#include "mapa.h"
#include "jugador.h"

#include <unordered_map>
#include <string>

class Juego {
    private:
        Mapa mapa;
        std::unordered_map<int, Jugador> jugadores;

    public:
        Juego(uint16_t ancho, uint16_t alto);

        void procesarComando(const int idJugador, const std::string& comando);
};

#endif
