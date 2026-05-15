#include "sender.h"

Sender::Sender(ProtocoloServer& protocolo, Queue<std::string>& clienteMensajes,
               const int idCliente):
        protocolo(protocolo),
        clienteMensajes(clienteMensajes),
        clienteConectado(true),
        idCliente(idCliente) {}

void Sender::run() {
    std::string mensaje;
    while (clienteConectado) {
        try {
            mensaje = clienteMensajes.pop();
        } catch (const ClosedQueue&) {
            clienteConectado = false;
            break;
        }

        int estadoEnvio = protocolo.enviarMensaje(mensaje, idCliente);

        if (estadoEnvio == 0) {
            // El cliente se desconectó
            break;
        }
    }
}

void Sender::stop() { clienteMensajes.close(); }
