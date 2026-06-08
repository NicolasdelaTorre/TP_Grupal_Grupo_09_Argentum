#ifndef ARGENTUM_COMMON_COMMON_BIOME_H
#define ARGENTUM_COMMON_COMMON_BIOME_H

#include <cstdint>
#include <string>

// La codificación del grid (grid_value <-> char) y la tabla de texturas de piso
// del grid viven en common_tiles: son un concepto de "tile/textura", separado
// del de "bioma" (una zona con criaturas). Se incluye acá para que el código que
// históricamente usaba esas funciones desde common_biome siga compilando.
#include "../common/common_tiles.h"

// Tipo de bioma. El valor numérico de un bioma coincide con el grid_value del
// tile de piso que le corresponde (ver common_tiles).
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

// Número del grid -> tipo de bioma.
BiomeType biome_from_cell(uint8_t cell);

// template_id del editor -> tipo.
BiomeType biome_from_template_id(const std::string& template_id);

// tipo -> template_id.
const char* biome_template_id(BiomeType biome);

#endif
