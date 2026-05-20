//
// Created by nicolas on 18/5/26.
//

#include "client_sender.h"

client_sender::client_sender(client_protocol& protocol, Queue<std::string>& events_queue):
        protocol(protocol), events_queue(events_queue) {}

void client_sender::run() {}
