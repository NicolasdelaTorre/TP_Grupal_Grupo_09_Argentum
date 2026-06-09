#ifndef ARGENTUM_COMMON_COMMON_BIOME_H
#define ARGENTUM_COMMON_COMMON_BIOME_H

#include <cstdint>
#include <string>

#include "../common/common_tiles.h"

enum class BiomeType : uint8_t {
    NONE = 0,  // sin bioma
    EXPLANADA = 1,
    BOSQUE_OSCURO = 2,
    BOSQUE_ARANAS = 3,
    DESIERTO = 4,
    CEMENTERIO = 5,
    NIEVE = 6,
    ROCOSAS = 7,
    PANTANO_EMBRUJADO = 8,
};

// template_id -> tipo.
BiomeType biome_from_template_id(const std::string& template_id);

#endif
