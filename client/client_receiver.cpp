//
// Created by nicolas on 18/5/26.
//

#include "client_receiver.h"

client_receiver::client_receiver(client_protocol& protocol, Queue<std::string>& server_queue):
        protocol(protocol), server_queue(server_queue) {}
void client_receiver::run() {}
