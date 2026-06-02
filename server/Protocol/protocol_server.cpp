#include "protocol_server.h"

#include <stdexcept>
#include <utility>

#include <sys/socket.h>

#include "../../common/liberror.h"
#include "../../common/message_types.h"
#include "../../common/protocol_util.h"

ProtocolServer::ProtocolServer(const char* port):
        socketServer(port), clientSockets(), clientCounter(0), mapRef(nullptr) {}

void ProtocolServer::setMap(const Map& map) { mapRef = &map; }

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
        // We utilize shutdown because there is a possibility that a message is being received
        // through the socket.
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

    switch (byteReceived) {
        case static_cast<uint8_t>(ClientMsg::USER_ARRIVAL):
            return returnUser(message, clientId);
        case static_cast<uint8_t>(ClientMsg::MOVEMENT):
            return returnMovement(message, clientId);
        case static_cast<uint8_t>(ClientMsg::SKIN_SELECTED):
            return returnSkin(message, clientId);
        case static_cast<uint8_t>(ClientMsg::TURN):
            return returnTurn(message, clientId);
        default:
            throw std::runtime_error("Protocol Error: unknown client's command");
    }
}

int ProtocolServer::returnSkin(std::string& message, const int clientId) {
    auto it = clientSockets.find(clientId);
    if (it == clientSockets.end()) {
        return 0;
    }
    uint8_t skinId = it->second.receive_byte();
    message += "skin.";
    message += std::to_string(skinId);
    return 1;
}

int ProtocolServer::returnTurn(std::string& message, const int clientId) {
    auto it = clientSockets.find(clientId);
    if (it == clientSockets.end()) {
        return 0;
    }
    uint8_t receivedByte = it->second.receive_byte();
    message += "turn.";
    switch (receivedByte) {
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
            throw std::runtime_error("Protocol Error: unknown turn direction");
    }
    return 1;
}

int ProtocolServer::returnUser(std::string& message, const int clientId) {
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

    switch (receivedByte) {
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
    // Convención: 1 = ok, 0 = cliente desconectado (matchea con el chequeo del Sender).
    try {
        auto it = clientSockets.find(clientId);
        if (it == clientSockets.end()) {
            return 0;  // cliente ya no está en el mapa de sockets
        }

        // LOGIN_OK trae spawn: parseamos "LOGIN_OK:x:y"
        if (message.rfind("LOGIN_OK", 0) == 0) {
            sendLoginOk(it->second, message);
        } else if (message == "LOGIN_FAIL") {
            it->second.sendByte(static_cast<uint8_t>(ServerMsg::LOGIN_FAIL));
        } else if (message == "FIRST_LOGIN") {
            it->second.sendByte(static_cast<uint8_t>(ServerMsg::FIRST_LOGIN));
        } else if (message == "MAP") {
            sendMap(it->second);
        } else if (message == "MOVE_OK") {
            it->second.sendByte(static_cast<uint8_t>(ServerMsg::MOVE_OK));
        } else if (message == "MOVE_FAIL") {
            it->second.sendByte(static_cast<uint8_t>(ServerMsg::MOVE_FAIL));
        } else if (message.rfind("NEW_PLAYER:", 0) == 0) {
            sendNewPlayer(it->second, message);
        } else if (message.rfind("PLAYER_MOVED:", 0) == 0) {
            sendPlayerMoved(it->second, message);
        } else if (message.rfind("PLAYER_DISCONNECTED:", 0) == 0) {
            sendPlayerDisconnected(it->second, message);
        } else if (message.rfind("STATS:", 0) == 0) {
            sendStats(it->second, message);
        } else {
            throw std::runtime_error("Protocol Error: unknown server's command: " + message);
        }

        return 1;
    } catch (const LibError& error) {
        if (ProtocolUtil::closedSocket(error))
            return 0;  // socket cerrado mid-send
        throw;
    }
}

void ProtocolServer::sendLoginOk(common_protocol& client, const std::string& message) {
    // Formato: "LOGIN_OK:<x>:<y>"
    size_t firstColon = message.find(':');
    size_t secondColon = message.find(':', firstColon + 1);
    if (firstColon == std::string::npos || secondColon == std::string::npos) {
        throw std::runtime_error("Protocol Error: malformed LOGIN_OK message: " + message);
    }
    int16_t x = static_cast<int16_t>(
            std::stoi(message.substr(firstColon + 1, secondColon - firstColon - 1)));
    int16_t y = static_cast<int16_t>(std::stoi(message.substr(secondColon + 1)));

    client.sendByte(static_cast<uint8_t>(ServerMsg::LOGIN_OK));
    client.send_two_bytes_number(static_cast<uint16_t>(x));
    client.send_two_bytes_number(static_cast<uint16_t>(y));
}

void ProtocolServer::sendNewPlayer(common_protocol& client, const std::string& message) {
    // Formato: "NEW_PLAYER:<id>:<x>:<y>:<dir>:<name>"
    size_t c1 = message.find(':');
    size_t c2 = message.find(':', c1 + 1);
    size_t c3 = message.find(':', c2 + 1);
    size_t c4 = message.find(':', c3 + 1);
    size_t c5 = message.find(':', c4 + 1);
    if (c1 == std::string::npos || c2 == std::string::npos || c3 == std::string::npos ||
        c4 == std::string::npos || c5 == std::string::npos) {
        throw std::runtime_error("Protocol Error: malformed NEW_PLAYER message: " + message);
    }
    uint16_t id = static_cast<uint16_t>(std::stoi(message.substr(c1 + 1, c2 - c1 - 1)));
    int16_t x = static_cast<int16_t>(std::stoi(message.substr(c2 + 1, c3 - c2 - 1)));
    int16_t y = static_cast<int16_t>(std::stoi(message.substr(c3 + 1, c4 - c3 - 1)));
    uint8_t dir = static_cast<uint8_t>(std::stoi(message.substr(c4 + 1, c5 - c4 - 1)));
    std::string name = message.substr(c5 + 1);

    client.sendByte(static_cast<uint8_t>(ServerMsg::NEW_PLAYER));
    client.send_two_bytes_number(id);
    client.send_two_bytes_number(static_cast<uint16_t>(x));
    client.send_two_bytes_number(static_cast<uint16_t>(y));
    client.sendByte(dir);
    client.send_two_bytes_number(static_cast<uint16_t>(name.size()));
    client.send_message(std::vector<char>(name.begin(), name.end()));
}

void ProtocolServer::sendPlayerMoved(common_protocol& client, const std::string& message) {
    // Formato: "PLAYER_MOVED:<id>:<x>:<y>:<dir>"
    size_t c1 = message.find(':');
    size_t c2 = message.find(':', c1 + 1);
    size_t c3 = message.find(':', c2 + 1);
    size_t c4 = message.find(':', c3 + 1);
    if (c1 == std::string::npos || c2 == std::string::npos || c3 == std::string::npos ||
        c4 == std::string::npos) {
        throw std::runtime_error("Protocol Error: malformed PLAYER_MOVED message: " + message);
    }
    uint16_t id = static_cast<uint16_t>(std::stoi(message.substr(c1 + 1, c2 - c1 - 1)));
    int16_t x = static_cast<int16_t>(std::stoi(message.substr(c2 + 1, c3 - c2 - 1)));
    int16_t y = static_cast<int16_t>(std::stoi(message.substr(c3 + 1, c4 - c3 - 1)));
    uint8_t dir = static_cast<uint8_t>(std::stoi(message.substr(c4 + 1)));

    client.sendByte(static_cast<uint8_t>(ServerMsg::PLAYER_MOVED));
    client.send_two_bytes_number(id);
    client.send_two_bytes_number(static_cast<uint16_t>(x));
    client.send_two_bytes_number(static_cast<uint16_t>(y));
    client.sendByte(dir);
}

void ProtocolServer::sendPlayerDisconnected(common_protocol& client, const std::string& message) {
    // Formato: "PLAYER_DISCONNECTED:<id>"
    size_t c1 = message.find(':');
    if (c1 == std::string::npos) {
        throw std::runtime_error("Protocol Error: malformed PLAYER_DISCONNECTED message: " +
                                 message);
    }
    uint16_t id = static_cast<uint16_t>(std::stoi(message.substr(c1 + 1)));

    client.sendByte(static_cast<uint8_t>(ServerMsg::PLAYER_DISCONNECTED));
    client.send_two_bytes_number(id);
}

void ProtocolServer::sendStats(common_protocol& client, const std::string& message) {
    // Formato: "STATS:<hp>:<maxHp>:<level>"
    size_t c1 = message.find(':');
    size_t c2 = message.find(':', c1 + 1);
    size_t c3 = message.find(':', c2 + 1);
    if (c1 == std::string::npos || c2 == std::string::npos || c3 == std::string::npos) {
        throw std::runtime_error("Protocol Error: malformed STATS message: " + message);
    }
    uint16_t hp = static_cast<uint16_t>(std::stoi(message.substr(c1 + 1, c2 - c1 - 1)));
    uint16_t maxHp = static_cast<uint16_t>(std::stoi(message.substr(c2 + 1, c3 - c2 - 1)));
    uint8_t level = static_cast<uint8_t>(std::stoi(message.substr(c3 + 1)));

    client.sendByte(static_cast<uint8_t>(ServerMsg::STATS_JUGADOR));
    client.send_two_bytes_number(hp);
    client.send_two_bytes_number(maxHp);
    client.sendByte(level);
}

void ProtocolServer::sendMap(common_protocol& client) {
    if (mapRef == nullptr) {
        throw std::runtime_error("Protocol Error: sendMap called but no Map registered");
    }
    const Map& map = *mapRef;

    client.sendByte(static_cast<uint8_t>(ServerMsg::MAP));
    client.send_two_bytes_number(map.getWidth());
    client.send_two_bytes_number(map.getHeight());
    client.send_two_bytes_number(map.getCellCount());

    for (size_t i = 0; i < map.getCellCount(); i++) {
        Cell cell = map.getCell(i);
        client.send_two_bytes_number(cell.textureId);
        client.send_two_bytes_number(cell.obstacleId);
        client.sendByte(cell.safeZone ? 1 : 0);
    }
}

void ProtocolServer::disconnectServer() { socketServer.shutdown(SHUT_RDWR); }

ProtocolServer::~ProtocolServer() {
    for (auto& socket: clientSockets) {
        socket.second.shutdown();
    }
    socketServer.close();
}
