//
// Created by nicolas on 18/5/26.
//

#include "client_sender.h"

client_sender::client_sender(client_protocol& protocol, Queue<Command>& events_queue):
        protocol(protocol), events_queue(events_queue) {}

void client_sender::run() {
        while(is_alive()) {
                try {
                Command data = events_queue.pop();
                protocol.send_message(data);
                } catch (const std::exception& err) {
                    std::cerr << "Unexpected exception: " << err.what() << "\n";
                } catch (...) {
                    std::cerr << "Unexpected exception: <unknown>\n";
                }
}
