//
// Created by nicolas on 18/5/26.
//

#include "client_receiver.h"

client_receiver::client_receiver(client_protocol& protocol, Queue<ServerMessageType>& server_queue):
        protocol(protocol), server_queue(server_queue) {}


void client_receiver::run() {
        while(is_alive()) {
            try {
                ServerMessageType data = protocol.receive_message();
                server_queue.push(data);
            } catch (const std::exception& err) {
                std::cerr << "Unexpected exception: " << err.what() << "\n";
            } catch (...) {
                std::cerr << "Unexpected exception: <unknown>\n";
            }
        }
}


bool client_receiver::is_alive() const {
    return _is_alive;
}