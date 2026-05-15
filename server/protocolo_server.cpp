#include "protocolo_server.h"

#include <utility>

#include <netinet/in.h>
#include <sys/socket.h>

#include "../common/liberror.h"

ProtocoloServer::ProtocoloServer(const char* puerto):
        socketServer(puerto), socketClientes(), contadorClientes(0) {}

int ProtocoloServer::esperarCliente() {
    try {
        Socket socketCliente = socketServer.accept();
        contadorClientes++;
        int idCliente = contadorClientes;

        // El id del cliente indica el orden de llegada al servidor.
        socketClientes.emplace(idCliente, std::move(socketCliente));

        return idCliente;
    } catch (const LibError&) {
        // Se cerro el socket del servidor.
        return 0;
    }
}

void ProtocoloServer::eliminarCliente(const int idCliente) {
    auto it = socketClientes.find(idCliente);
    if (it != socketClientes.end()) {
        // Se utiliza shutdown porque existe la posibilidad de que se este intentado recibir un
        // mensaje a traves del socket.
        it->second.shutdown(SHUT_RDWR);
        socketClientes.erase(it);
    }
}

int ProtocoloServer::recibirMensaje(std::string& mensaje, const int idCliente) {
    std::vector<char> mensajeRecibido(1);
    auto it = socketClientes.find(idCliente);
    if (it == socketClientes.end()) {
        return 0;
    }

    int bytesRecibidos = it->second.recvall(mensajeRecibido.data(), 1);

    if (bytesRecibidos != 1) {
        // Se cerro el socket del cliente.
        return 0;
    }

    // Implementar logica segun los comandos de los clientes
    switch (mensajeRecibido[0]) {
        case 1:
            // Para que no se queje el compilador
            mensaje.push_back(mensajeRecibido[0]);
            return 0;
        default:
            // Codigo no reconocido
            return 0;
    }
}

int ProtocoloServer::enviarMensaje(const std::string& mensaje, const int idCliente) {
    auto it = socketClientes.find(idCliente);
    if (it == socketClientes.end()) {
        return 0;
    }

    size_t posicionAccion = mensaje.find(':');

    if (posicionAccion == std::string::npos) {
        // El mensaje no tiene el formato correcto.
        return 0;
    }

    std::string accion = mensaje.substr(0, posicionAccion);

    std::vector<char> mensajeAEnviar;

    int bytesEnviados = it->second.sendall(mensajeAEnviar.data(), mensajeAEnviar.size());

    if ((size_t)bytesEnviados != mensajeAEnviar.size()) {
        // Se cerro el socket del cliente.
        return 0;
    }

    return 1;
}

void ProtocoloServer::desconectarServidor() { socketServer.shutdown(SHUT_RDWR); }

ProtocoloServer::~ProtocoloServer() {
    for (auto& socket: socketClientes) {
        socket.second.close();
    }
    socketServer.close();
}
