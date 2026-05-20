//
// Created by nicolas on 17/5/26.
//

#include "common_protocol.h"
#include <utility>


common_protocol::common_protocol(Socket skt): skt(std::move(skt)) {}