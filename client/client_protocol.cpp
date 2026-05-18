//
// Created by nicolas on 17/5/26.
//

#include "client_protocol.h"
#include <utility>

client_protocol::client_protocol(Socket skt) : protocol(std::move(skt)){}
