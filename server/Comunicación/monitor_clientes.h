#ifndef MONITOR_CLIENTES_H
#define MONITOR_CLIENTES_H

#include <map>
#include <string>

#include "../../common/queue.h"

class MonitorClientes {
private:
    std::map<int, Queue<std::string>*> Clientes;
    std::mutex mtx;

public:
    MonitorClientes();

    void agregarQueues(int idCliente, Queue<std::string>& queueCliente);

    void eliminarQueue(const int idCliente);

    void broadcast(const std::string& mensaje);

    // Envía un mensaje solo al cliente con ese id. No hace nada si el id no existe.
    void enviarACliente(int idCliente, const std::string& mensaje);
};

#endif
