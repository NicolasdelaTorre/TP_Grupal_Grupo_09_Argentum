#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <string>

#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../Comunicación/monitor_clientes.h"

#include "juego.h"

class Gameloop: public Thread {
private:
    Queue<std::string>& comandos;
    MonitorClientes& queuesClientes;
    bool juegoTerminado;
    Juego juego;

public:
    Gameloop(Queue<std::string>& comandos, MonitorClientes& queuesClientes);

    virtual void run() override;

    virtual void stop() override;
};

#endif
