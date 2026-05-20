#include "juego.h"

#include <iostream>
#include <stdexcept>

Juego::Juego(uint16_t ancho, uint16_t alto): mapa(ancho, alto) {}

void Juego::procesarComando(const int idJugador, const std::string& comando) {
    size_t pocisionComando = comando.find('.');
    if (pocisionComando == std::string::npos) {
        throw std::runtime_error("Error Juego: comando del cliente mal formado");
    }

    std::string tipoDato = comando.substr(0, pocisionComando);

    if (tipoDato == "usuario") {
        std::string usuario = comando.substr(pocisionComando + 1);
        jugadores.emplace(idJugador, Jugador(usuario));
        mapa.agregarJugador();
        std::cout << "Hola " << usuario << std::endl;
    } else if (tipoDato == "movimiento") {
        procesarMovimiento(idJugador, comando, pocisionComando);
    }
}

void Juego::procesarMovimiento(const int idJugador, const std::string& comando,
                               size_t pocisionComando) {
    auto itJugador = jugadores.find(idJugador);
    if (itJugador == jugadores.end()) {
        throw std::runtime_error("Error Juego: jugador no encontrado");
    }

    size_t pocisionDireccion = comando.find('.', pocisionComando + 1);
    if (pocisionDireccion == std::string::npos) {
        throw std::runtime_error("Error Juego: comando del cliente mal formado");
    }

    std::string direccion =
            comando.substr(pocisionComando + 1, pocisionDireccion - pocisionComando - 1);
    if (mapa.moverJugador(direccion, itJugador->second.getX(), itJugador->second.getY())) {
        // algo deberiamos hacer para mostrar algo en el juego
        return;
    }

    itJugador->second.cambiarPosicion(direccion);
}
