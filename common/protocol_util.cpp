#include "protocol_util.h"

#include <stdexcept>

#include <netinet/in.h>

bool ProtocolUtil::closedSocket(const LibError& error) {
    std::string messageError = error.what();

    return messageError.find("socket sent only") != std::string::npos ||
           messageError.find("socket received only") != std::string::npos ||
           messageError.find("socket accept failed") != std::string::npos;
}

uint16_t ProtocolUtil::readLength(const std::vector<char>& bytes) {
    return htons((uint16_t)bytes[0]) + bytes[1];
}

void ProtocolUtil::writeLength(uint16_t length, std::vector<char>& bytes) {
    bytes.push_back((uint8_t)htons(length));
    bytes.push_back((uint8_t)length);
}
