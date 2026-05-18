#ifndef SERVER_H
#define SERVER_H

#include <string>

#include "../common/queue.h"
#include "Comunicación/aceptador.h"
#include "Comunicación/monitor_clientes.h"
#include "Logica/gameloop.h"
#include "Protocolo/protocolo_server.h"

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
