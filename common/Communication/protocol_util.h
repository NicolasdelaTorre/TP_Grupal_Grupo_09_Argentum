#ifndef PROTOCOL_COMUN_H
#define PROTOCOL_COMUN_H

#include <cstdint>
#include <string>
#include <vector>

#include "liberror.h"

class ProtocolUtil {
public:
    static bool closedSocket(const LibError& error);

    static uint16_t readLength(const std::vector<char>& bytes);

    static void writeLength(uint16_t length, std::vector<char>& bytes);
};

#endif
