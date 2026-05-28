#include "receiver.h"

Receiver::Receiver(ProtocolServer& protocol, Queue<std::string>& commands, const int idCliente):
        protocol(protocol), commands(commands), idCliente(idCliente) {}

void Receiver::run() {
    std::string message;
    while (protocol.receiveMessage(message, idCliente)) {
        commands.push(std::to_string(idCliente) + ":" + message);
        message.clear();
    }
    // El socket se cerró — avisamos al gameloop para que notifique a los demás.
    commands.push(std::to_string(idCliente) + ":disconnect.");
}
