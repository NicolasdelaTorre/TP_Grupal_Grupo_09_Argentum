//
// Created by nicolas on 17/5/26.
//

#include "common_protocol.h"

#include <sys/socket.h>
#include <utility>


common_protocol::common_protocol(Socket skt): skt(std::move(skt)) {}

int common_protocol::send(const void* data, size_t size) { return skt.sendall(data, size); }

int common_protocol::recv(void* data, size_t size) {
    return skt.recvall(data, static_cast<unsigned int>(size));
}

void common_protocol::close() { skt.shutdown(SHUT_RDWR); }
