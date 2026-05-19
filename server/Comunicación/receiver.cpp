#include "receiver.h"

Receiver::Receiver(ProtocoloServer& protocolo, Queue<std::string>& comandos, const int idCliente):
        protocolo(protocolo), comandos(comandos), idCliente(idCliente) {}

void Receiver::run() {
    std::string mensaje;
    while (protocolo.recibirMensaje(mensaje, idCliente)) {
        comandos.push(std::to_string(idCliente) + ":" + mensaje);
        mensaje.clear();
    }
}
