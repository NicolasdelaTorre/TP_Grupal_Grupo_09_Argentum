#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <atomic>
#include <string>

#include "../../common/queue.h"

#include "monitor_clientes.h"
#include "../Protocolo/protocolo_server.h"
#include "receiver.h"
#include "sender.h"

class ClientHandler {
private:
    Queue<std::string> queueCliente;
    Sender sender;
    Receiver receiver;
    std::atomic<bool> clienteConectado;

public:
    const int idCliente;
    /*
     * Crea un objeto para recibir mensajes del cliente y otro para enviar mensajes por parte del
     * servidor.
     */
    ClientHandler(ProtocoloServer& protocolo, Queue<std::string>& comandos,
                  MonitorClientes& queueCliente, const int idCliente);

    bool clienteDesconectado();

    void iniciarHilos();

    /*
     * Fuerza el cierre de los dos hilos.
     */
    void eliminarCliente();

    /*
     * Espera a que terminen los hilos que envian y reciben mensajes del cliente.
     */
    ~ClientHandler();
};

#endif
