#ifndef SERVER_H
#define SERVER_H

#include <string>

#include "../common/queue.h"

#include "aceptador.h"
#include "gameloop.h"
#include "monitor_clientes.h"
#include "protocolo_server.h"

class Servidor {
private:
    ProtocoloServer protocolo;
    Queue<std::string> comandos;
    MonitorClientes queuesClientes;
    Gameloop gameloop;
    Aceptador aceptador;

public:
    explicit Servidor(const char* puerto);

    /*
     * Comienza el game loop y la aceptación de clientes.
     */
    void empezarJuego();
};

#endif
