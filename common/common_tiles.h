#ifndef ARGENTUM_COMMON_COMMON_TILES_H
#define ARGENTUM_COMMON_COMMON_TILES_H

#include <cstdint>
#include <string>
#include <vector>

// Tiles de piso del grid del mapa.
//
// El grid del mapa guarda, por celda, un único valor (`grid_value`) que indica
// qué textura de piso va en esa celda. 
// su valor en el grid, un id legible, la textura
// (ruta relativa al directorio de imágenes de common; vacía si el tile solo
// tiñe con color) y el color de respaldo.
struct FloorTile {
    uint8_t grid_value;
    const char* id;
    const char* texture;  // relativa a common/assets/images; "" si no tiene
    const char* color;    // color hex
};

// Tile "exterior
constexpr uint8_t EXTERIOR_TILE_VALUE = 255;

// Tabla completa de tiles de piso (pisos de bioma + tiles independientes).
const std::vector<FloorTile>& floor_tiles();

// Busca el tile cuyo `grid_value` coincide. Devuelve nullptr si no existe.
const FloorTile* floor_tile_from_grid_value(uint8_t grid_value);

// Busca el tile por su id. Devuelve nullptr si no existe.
const FloorTile* floor_tile_from_id(const std::string& id);

// Busca el tile cuya textura (ruta relativa) coincide. Devuelve nullptr si no
// existe o si la textura está vacía. Lo usa el loader para resolver el piso de
// un environment a partir de su `floor_texture`.
const FloorTile* floor_tile_from_texture(const std::string& texture);

// de un valor de celda del grid a un único carácter.
char grid_value_to_char(uint8_t value);

// Inversa de grid_value_to_char. Caracteres inválidos devuelven 0.
uint8_t grid_char_to_value(char c);

#endif
