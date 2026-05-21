#include "gameloop.h"

Gameloop::Gameloop(Queue<std::string>& comandos, MonitorClientes& queuesClientes):
        comandos(comandos), queuesClientes(queuesClientes), juegoTerminado(false), juego(10, 10) {}

void Gameloop::run() {
    while (!juegoTerminado) {
        std::string comando;
        while (comandos.try_pop(comando)) {
            size_t posId = comando.find(':');
            int idJugador = std::stoi(comando.substr(0, posId));
            std::string cmd = comando.substr(posId + 1);

            juego.procesarComando(idJugador, cmd);

            if (cmd.substr(0, 7) == "usuario") {
                queuesClientes.enviarACliente(idJugador, "LOGIN_OK");
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void Gameloop::stop() { juegoTerminado = true; }
