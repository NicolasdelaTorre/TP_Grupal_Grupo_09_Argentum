#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

#include <map>
#include <string>
#include <vector>

#include "../../common/liberror.h"
#include "../../common/socket.h"
#include "../../common/common_protocol.h"

class ProtocolServer {
private:
    Socket socketServer;
    std::map<int, common_protocol> clientSockets;
    int clientCounter;

    /*
     * Deserializes the message to send the newly arrived user to the server. 
     * Returns 1 on success or 0 if the client's socket was closed.
     */
    int returnUser(std::string& message, const int clientId);

    /*
     * Deserializes the message to send the client's movement to the server.
     * Returns 1 on success or 0 if the client's socket was closed.
     */
    int returnMovement(std::string& message, const int clientId);

public:
    explicit ProtocolServer(const char* port);

    /*
     * Keep waiting for a client to arrive or until the server's socket is closed. 
     * Returns the client's id or 0 if the server's socket was closed.
     */
    int waitClient();

    void deleteClient(const int clientId);

    /*
     * Recieves a message from the client and saves the important data in the string with the format: data1.data2 . 
     * Returns 1 on success or 0 if the client's socket was closed.
     */
    int receiveMessage(std::string& message, const int clientId);

    /*
     * Serializes a message to send to the client.
     * Returns 1 on success or 0 if the client's socket was closed.
     */
    int sendMessage(const std::string& message, const int clientId);

    void disconnectServer();

    // Close all client sockets and the server socket.
    ~ProtocolServer();
};

#endif
