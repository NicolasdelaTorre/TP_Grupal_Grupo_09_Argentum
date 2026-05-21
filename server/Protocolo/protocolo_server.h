#ifndef PROTOCOLO_SERVER_H
#define PROTOCOLO_SERVER_H

#include <map>
#include <string>
#include <vector>

#include "../../common/liberror.h"
#include "../../common/socket.h"
#include "../../common/common_protocol.h"

class ProtocoloServer {
private:
    Socket socketServer;
    std::map<int, common_protocol> socketClientes;
    int contadorClientes;

    /*
     * Deserializa el mensaje para enviar el usuario recien llegado al servidor.
     * Devuelve 1 en caso de exito o 0 si se cerro el socket del cliente.
     */
    int devolverUsuario(std::string& mensaje, const int idCliente);

    /*
     * Deserializa el mensaje para enviar el movimiento del cliente al servidor.
     * Devuelve 1 en caso de exito o 0 si se cerro el socket del cliente.
     */
    int devolverMovimiento(std::string& mensaje, const int idCliente);

public:
    explicit ProtocoloServer(const char* puerto);

    /*
     * Se queda esperando a la llegada de un cliente o hasta que se cierre el
     * socket del servidor. Devuelve el id del cliente o 0 si se cerró el socket
     * del servidor.
     */
    int esperarCliente();

    void eliminarCliente(const int idCliente);

    /*
     * Recibe un mensaje del cliente y guarda los datos importantes en el string
     * con el formato: dato1.dato2 . Devuelve 1 en caso de exito o 0 si se cerro
     * el socket del cliente.
     */
    int recibirMensaje(std::string& mensaje, const int idCliente);

    /*
     * Serializa un menasaje para enviarselo al cliente.
     * Devuelve 1 en caso de exito o 0 si se cerro el socket del cliente.
     */
    int enviarMensaje(const std::string& mensaje, const int idCliente);

    void desconectarServidor();

    // Cierra todos los sockets de los clientes y del servidor.
    ~ProtocoloServer();
};

#endif
