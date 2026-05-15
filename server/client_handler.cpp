#include "client_handler.h"

ClientHandler::ClientHandler(ProtocoloServer& protocolo, Queue<std::string>& comandos,
                             MonitorClientes& queuesClientes, const int idCliente):
        queueCliente(),
        sender(protocolo, queueCliente, idCliente),
        receiver(protocolo, comandos, idCliente),
        clienteConectado(true),
        idCliente(idCliente) {
    queuesClientes.agregarQueues(idCliente, queueCliente);
}

bool ClientHandler::clienteDesconectado() { return !clienteConectado; }

void ClientHandler::iniciarHilos() {
    sender.start();
    receiver.start();
}

void ClientHandler::eliminarCliente() {
    sender.stop();
    receiver.stop();
}

ClientHandler::~ClientHandler() {
    sender.join();
    receiver.join();
}
