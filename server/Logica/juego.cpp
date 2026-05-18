#include "juego.h"

#include <stdexcept>

Juego::Juego(uint16_t ancho, uint16_t alto) : mapa(ancho, alto) {}

void Juego::procesarComando(const int idJugador, const std::string& comando) {
    size_t pocisionComando = comando.find('.');
    if (pocisionComando == std::string::npos) {
        throw std::runtime_error("Error Juego: comando del cliente mal formado");
    }

    std::string tipoDato = comando.substr(0, pocisionComando);

    if (tipoDato == "usuario") {
        std::string usuario = comando.substr(pocisionComando + 1);
        jugadores.emplace(idJugador, Jugador(usuario));
        //mapa.agregarJugador();
    }
}
