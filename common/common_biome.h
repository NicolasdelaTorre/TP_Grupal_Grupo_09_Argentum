#ifndef ARGENTUM_COMMON_COMMON_BIOME_H
#define ARGENTUM_COMMON_COMMON_BIOME_H

#include <cstdint>
#include <string>

// Tipo de bioma. El valor numérico es lo que se persiste en el grid del mapa
enum class BiomeType : uint8_t {
    NONE = 0,  // sin bioma
    EXPLANADA = 1,
    BOSQUE_OSCURO = 2,
    BOSQUE_ARANAS = 3,
    DESIERTO = 4,
    CEMENTERIO = 5,
    CUEVAS_ORCOS = 6,
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
