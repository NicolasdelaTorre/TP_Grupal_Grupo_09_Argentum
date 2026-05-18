#include "aceptador.h"

Aceptador::Aceptador(ProtocoloServer& protocolo, Queue<std::string>& comandos,
                     MonitorClientes& queuesClientes):
        protocolo(protocolo), clientes(), comandos(comandos), queuesClientes(queuesClientes) {}

void Aceptador::run() {
    int idCliente;
    while ((idCliente = protocolo.esperarCliente())) {
        auto* cliente = new ClientHandler(protocolo, comandos, queuesClientes, idCliente);
        reap();
        clientes.push_back(cliente);
        cliente->iniciarHilos();
    }
    clear();
}

void Aceptador::reap() {
    clientes.remove_if([this](const auto& cliente) {
        bool clienteDesconectado = cliente->clienteDesconectado();
        if (clienteDesconectado) {
            protocolo.eliminarCliente(cliente->idCliente);
            queuesClientes.eliminarQueue(cliente->idCliente);
            delete cliente;
        }
        return clienteDesconectado;
    });
}

void Aceptador::clear() {
    for (auto* cliente: clientes) {
        cliente->eliminarCliente();
        protocolo.eliminarCliente(cliente->idCliente);
        queuesClientes.eliminarQueue(cliente->idCliente);
        delete cliente;
    }

    clientes.clear();
}

void Aceptador::stop() {
    clear();
    protocolo.desconectarServidor();
}
