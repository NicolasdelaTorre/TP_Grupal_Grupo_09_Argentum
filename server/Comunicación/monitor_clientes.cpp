#include "monitor_clientes.h"

MonitorClientes::MonitorClientes(): Clientes(), mtx() {}

void MonitorClientes::agregarQueues(const int idCliente, Queue<std::string>& queueCliente) {
    std::lock_guard<std::mutex> lock(mtx);
    Clientes[idCliente] = &queueCliente;
}

void MonitorClientes::eliminarQueue(const int idCliente) {
    std::lock_guard<std::mutex> lock(mtx);
    Clientes.erase(idCliente);
}

void MonitorClientes::broadcast(const std::string& mensaje) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& cliente: Clientes) {
        cliente.second->push(mensaje);
    }
}
