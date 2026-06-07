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

// Codificación de un valor de celda del grid a un único carácter. Los biomas
// usan los dígitos 0-8; los modificadores de piso usan valores superiores.
// Para mantener un carácter por celda se usa base 36: 0-9 y luego a-z
// (valores 10-35). Valores fuera de rango devuelven '0'.
char grid_value_to_char(uint8_t value);

// Inversa de grid_value_to_char. Caracteres inválidos devuelven 0.
uint8_t grid_char_to_value(char c);

#endif
