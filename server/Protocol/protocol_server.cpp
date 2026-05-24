#include "protocol_server.h"

#include <stdexcept>
#include <utility>

#include <sys/socket.h>

#include "../../common/liberror.h"
#include "../../common/message_types.h"
#include "../../common/protocol_util.h"

ProtocolServer::ProtocolServer(const char* port):
        socketServer(port), clientSockets(), clientCounter(0) {}

int ProtocolServer::waitClient() {
    try {
        Socket clientSocket = socketServer.accept();
        clientCounter++;
        int clientId = clientCounter;
        
        // The client's id indicates the order of arrival to the server.
        clientSockets.emplace(clientId, common_protocol(std::move(clientSocket)));

        return clientId;
    } catch (const LibError& error) {
        if (ProtocolUtil::closedSocket(error))
            return 0;
        throw;
    }
}

void ProtocolServer::deleteClient(const int clientId) {
    auto it = clientSockets.find(clientId);
    if (it != clientSockets.end()) {
        // We utilize shutdown because there is a possibility that a message is being received through the socket.
        it->second.shutdown();
        clientSockets.erase(it);
    }
}

int ProtocolServer::receiveMessage(std::string& message, const int clientId) {
    std::vector<char> messageReceived(1);
    auto it = clientSockets.find(clientId);
    if (it == clientSockets.end()) {
        // The client has disconnected
        return 0;
    }

    u_int8_t byteReceived = it->second.receive_byte();

    if (!byteReceived) {
        // The client has closed the socket.
        return 0;
    }

    switch (messageReceived[0]) {
        case static_cast<uint8_t>(ClientMsg::USER_ARRIVAL):
            return returnUser(message, clientId);
        case static_cast<uint8_t>(ClientMsg::MOVEMENT):
            return returnMovement(message, clientId);
        default:
            throw std::runtime_error("Protocol Error: unknown client's command");
    }
}

int ProtocolServer::returnUser(std::string& message, const int clientId) {
    std::vector<char> messageReceived(2);
    auto it = clientSockets.find(clientId);
    if (it == clientSockets.end()) {
        // The client has disconnected
        return 0;
    }

    u_int16_t nameLenght = it->second.receive_two_bytes_number();

    if (!nameLenght) {
        // The client has closed the socket.
        return 0;
    }

    std::string userName = it->second.receive_message(nameLenght);

    message += "user.";
    message += userName;

    return 1;
}

int ProtocolServer::returnMovement(std::string& message, const int clientId) {
    message += "movement.";
    std::vector<char> messageReceived(1);

    auto it = clientSockets.find(clientId);
    if (it == clientSockets.end()) {
        // The client has disconnected
        return 0;
    }

    u_int8_t receivedByte = it->second.receive_byte();

    if (!receivedByte) {
        // The client has closed the socket.
        return 0;
    }

    uint8_t direction = messageReceived[0];
    switch (direction) {
        case static_cast<uint8_t>(ClientMsg::TOP):
            message += "top";
            break;
        case static_cast<uint8_t>(ClientMsg::BOTTOM):
            message += "bottom";
            break;
        case static_cast<uint8_t>(ClientMsg::LEFT):
            message += "left";
            break;
        case static_cast<uint8_t>(ClientMsg::RIGHT):
            message += "right";
            break;
        default:
            throw std::runtime_error("Protocol Error: unknown client's direction");
    }

    return 1;
}

int ProtocolServer::sendMessage(const std::string& message, const int clientId) {
    try {
        auto it = clientSockets.find(clientId);
        if (it == clientSockets.end()) {
            // The client has disconnected
            return 0;
        }

        std::vector<uint8_t> buffer;

        if (message == "LOGIN_OK") {
            buffer.push_back(static_cast<uint8_t>(ServerMsg::LOGIN_OK));
        } else if (message == "LOGIN_FAIL") {
            buffer.push_back(static_cast<uint8_t>(ServerMsg::LOGIN_FAIL));
        } else {
            // Unknown message type. Ignore without throwing an exception.
            return 1;
        }

        std::vector<char> messageToSend;

        it->second.send_message(message);

        return 0;
    } catch (const LibError& error) {
        if (ProtocolUtil::closedSocket(error))
            return 1;
        throw;
    }
}

void ProtocolServer::disconnectServer() { socketServer.shutdown(SHUT_RDWR); }

ProtocolServer::~ProtocolServer() {
    for (auto& socket: clientSockets) {
        socket.second.shutdown();
    }
    socketServer.close();
}
