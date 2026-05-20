//
// Created by nicolas on 17/5/26.
//

#include "common_protocol.h"

#include <utility>


common_protocol::common_protocol(Socket skt): skt(std::move(skt)) {}

int common_protocol::send(const char* data, size_t size) { return skt.sendall(data, size); }
