#include "protocolo_util.h"

#include <stdexcept>

#include <netinet/in.h>

bool ProtocoloUtil::socketCerrado(const LibError& error) {
    std::string errorMensaje = error.what();

    return errorMensaje.find("socket sent only") != std::string::npos ||
           errorMensaje.find("socket received only") != std::string::npos ||
           errorMensaje.find("socket accept failed") != std::string::npos;
}

uint16_t ProtocoloUtil::leerLongitud(const std::vector<char>& bytes) {
    return htons((uint16_t)bytes[0]) + bytes[1];
}

void ProtocoloUtil::escribirLongitud(uint16_t longitud, std::vector<char>& bytes) {
    bytes.push_back((uint8_t)htons(longitud));
    bytes.push_back((uint8_t)longitud);
}
