#include "servidor.h"

Servidor::Servidor(const char* puerto):
        protocolo(puerto),
        comandos(),
        queuesClientes(),
        gameloop(comandos, queuesClientes),
        aceptador(protocolo, comandos, queuesClientes) {}

void Servidor::empezarJuego() {
    gameloop.start();
    aceptador.start();

    // Esperamos a que el usuario quiera terminar el juego presionando la tecla 'q
    while (std::getchar() != 'q') {}

    gameloop.stop();
    aceptador.stop();

    gameloop.join();
    aceptador.join();
}
