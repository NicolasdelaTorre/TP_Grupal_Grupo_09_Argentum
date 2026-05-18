#include "gameloop.h"

Gameloop::Gameloop(Queue<std::string>& comandos, MonitorClientes& queuesClientes):
        comandos(comandos), queuesClientes(queuesClientes), juegoTerminado(false), juego(10, 10) {}

void Gameloop::run() {
    while (!juegoTerminado) {
        std::string comando;
        while (comandos.try_pop(comando)) {}

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void Gameloop::stop() { juegoTerminado = true; }
