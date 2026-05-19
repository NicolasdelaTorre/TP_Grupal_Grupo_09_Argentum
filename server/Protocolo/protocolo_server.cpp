#include "protocolo_server.h"

#include <stdexcept>
#include <utility>

#include <sys/socket.h>

#include "../../common/liberror.h"
#include "../../common/protocolo_util.h"

#include "comandos.h"

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
    } catch (const LibError& error) {
        if (ProtocoloUtil::socketCerrado(error))
            return 0;
        throw;
    }
}

void ProtocoloServer::eliminarCliente(const int idCliente) {
    auto it = socketClientes.find(idCliente);
    if (it != socketClientes.end()) {
        // Se utiliza shutdown porque existe la posibilidad de que se este intentado
        // recibir un mensaje a traves del socket.
        it->second.shutdown(SHUT_RDWR);
        socketClientes.erase(it);
    }
}

int ProtocoloServer::recibirMensaje(std::string& mensaje, const int idCliente) {
    std::vector<char> mensajeRecibido(1);
    auto it = socketClientes.find(idCliente);
    if (it == socketClientes.end()) {
        // El cliente se desconecto
        return 0;
    }

    int bytesRecibidos = it->second.recvall(mensajeRecibido.data(), 1);

    if (!bytesRecibidos) {
        // Se cerro el socket del cliente.
        return 0;
    }

    switch (mensajeRecibido[0]) {
        case static_cast<uint8_t>(Comando::LLEGADA_USUARIO):
            return devolverUsuario(mensaje, idCliente);
        case static_cast<uint8_t>(Comando::MOVIMIENTO):
            return devolverMovimiento(mensaje, idCliente);
        default:
            throw std::runtime_error("Error Protocolo: comando del cliente desconocido");
    }
}

int ProtocoloServer::devolverUsuario(std::string& mensaje, const int idCliente) {
    std::vector<char> mensajeRecibido(2);
    auto it = socketClientes.find(idCliente);
    if (it == socketClientes.end()) {
        // El cliente se desconecto
        return 0;
    }

    int bytesRecibidos = it->second.recvall(mensajeRecibido.data(), 2);

    if (!bytesRecibidos) {
        // Se cerro el socket del cliente.
        return 0;
    }

    uint16_t longitudNombre = ProtocoloUtil::leerLongitud(mensajeRecibido);

    mensajeRecibido.resize(longitudNombre);

    bytesRecibidos = it->second.recvall(mensajeRecibido.data(), longitudNombre);

    if (!bytesRecibidos) {
        // Se cerro el socket del cliente.
        return 0;
    }

    mensaje += "usuario.";
    mensaje.assign(mensajeRecibido.begin(), mensajeRecibido.end());

    return 1;
}

int ProtocoloServer::devolverMovimiento(std::string& mensaje, const int idCliente) {
    mensaje += "movimiento.";
    std::vector<char> mensajeRecibido(1);

    auto it = socketClientes.find(idCliente);
    if (it == socketClientes.end()) {
        // El cliente se desconecto
        return 0;
    }

    int bytesRecibidos = it->second.recvall(mensajeRecibido.data(), 1);

    if (!bytesRecibidos) {
        // Se cerro el socket del cliente.
        return 0;
    }

    uint8_t direccion = mensajeRecibido[0];
    switch (direccion) {
        case static_cast<uint8_t>(Comando::ARRIBA):
            mensaje += "arriba";
            break;
        case static_cast<uint8_t>(Comando::ABAJO):
            mensaje += "abajo";
            break;
        case static_cast<uint8_t>(Comando::IZQUIERDA):
            mensaje += "izquierda";
            break;
        case static_cast<uint8_t>(Comando::DERECHA):
            mensaje += "derecha";
            break;
        default:
            throw std::runtime_error("Error Protocolo: direccion del cliente desconocida");
    }

    return 1;
}

int ProtocoloServer::enviarMensaje(const std::string& mensaje, const int idCliente) {
    try {
        auto it = socketClientes.find(idCliente);
        if (it == socketClientes.end()) {
            // Se cerro el socket del cliente
            return 0;
        }

        size_t posicionAccion = mensaje.find(':');

        if (posicionAccion == std::string::npos) {
            throw std::runtime_error(
                    "Error Protocolo: mensaje del servidor con formato incorrecto");
        }

        std::vector<char> mensajeAEnviar;

        int bytesEnviados = it->second.sendall(mensajeAEnviar.data(), mensajeAEnviar.size());

        if (!bytesEnviados) {
            // Se cerro el socket del cliente.
            return 0;
        }

        return 1;
    } catch (const LibError& error) {
        if (ProtocoloUtil::socketCerrado(error))
            return 0;
        throw;
    }
}

void ProtocoloServer::desconectarServidor() { socketServer.shutdown(SHUT_RDWR); }

ProtocoloServer::~ProtocoloServer() {
    for (auto& socket: socketClientes) {
        socket.second.close();
    }
    socketServer.close();
}
