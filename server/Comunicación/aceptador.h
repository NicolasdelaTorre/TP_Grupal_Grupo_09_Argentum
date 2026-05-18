#ifndef ACEPTADOR_H
#define ACEPTADOR_H

#include <list>
#include <string>

#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../Protocolo/protocolo_server.h"

#include "client_handler.h"
#include "monitor_clientes.h"

class Aceptador: public Thread {
private:
    ProtocoloServer& protocolo;
    std::list<ClientHandler*> clientes;
    Queue<std::string>& comandos;
    MonitorClientes& queuesClientes;

    /*
     * Verifica si un cliente se fue del juego. En ese caso, se debe eliminar el
     * clientHandler de la lista de clientes.
     */
    void reap();

    /*
     * Elimina todos los clientes de la lista.
     */
    void clear();

public:
    Aceptador(ProtocoloServer& protocolo, Queue<std::string>& comandos,
              MonitorClientes& queuesClientes);

    virtual void run() override;

    virtual void stop() override;
};

#endif
