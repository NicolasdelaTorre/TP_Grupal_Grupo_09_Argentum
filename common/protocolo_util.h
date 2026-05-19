#ifndef PROTOCOLO_COMUN_H
#define PROTOCOLO_COMUN_H

#include <cstdint>
#include <string>
#include <vector>

#include "liberror.h"

class ProtocoloUtil {
public:
    static bool socketCerrado(const LibError& error);

    static uint16_t leerLongitud(const std::vector<char>& bytes);

    static void escribirLongitud(uint16_t longitud, std::vector<char>& bytes);
};

#endif
