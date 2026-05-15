#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <string>

#include "../common/queue.h"
#include "../common/thread.h"

#include "monitor_clientes.h"

class Gameloop: public Thread {
private:
    Queue<std::string>& comandos;
    MonitorClientes& queuesClientes;
    bool juegoTerminado;

public:
    Gameloop(Queue<std::string>& comandos, MonitorClientes& queuesClientes);

    virtual void run() override;

    virtual void stop() override;
};

#endif
