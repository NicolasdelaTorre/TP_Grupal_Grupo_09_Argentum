#include "common_tiles.h"

namespace {

const std::vector<FloorTile> FLOOR_TILES = {
        {1, "explanada", "tile_pasto_128x128.png", "#A8C97F"},
        {2, "bosque_oscuro", "", "#2E4A2E"},
        {3, "bosque_aranas", "", "#5B8C5A"},
        {4, "desierto", "tile_desierto_128x128.png", "#D6D6D6"},
        {5, "cementerio", "", "#7D8C8C"},
        {6, "cuevas_orcos", "", "#A67B5B"},
        {7, "rocosas", "", "#888B8D"},
        {8, "pantano_embrujado", "", "#6B8E23"},
        {9, "adoquin", "tiles/adoquin.png", "#9A9A9A"},
        {10, "adoquin_oscuro", "tiles/adoquin_oscuro.png", "#5C5C5C"},
        {11, "madera", "tiles/madera.png", "#7A4B25"},
        {EXTERIOR_TILE_VALUE, "exterior", "", "#000000"},
};

}  // namespace

const std::vector<FloorTile>& floor_tiles() { return FLOOR_TILES; }

const FloorTile* floor_tile_from_grid_value(uint8_t grid_value) {
    for (const auto& tile: FLOOR_TILES) {
        if (tile.grid_value == grid_value) {
            return &tile;
        }
    }
    return nullptr;
}

const FloorTile* floor_tile_from_id(const std::string& id) {
    for (const auto& tile: FLOOR_TILES) {
        if (id == tile.id) {
            return &tile;
        }
    }
    return nullptr;
}

char grid_value_to_char(uint8_t value) {
    if (value < 10) {
        return static_cast<char>('0' + value);
    }
    if (value < 36) {
        return static_cast<char>('a' + (value - 10));
    }
    return '0';
}

uint8_t grid_char_to_value(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(c - '0');
    }
    if (c >= 'a' && c <= 'z') {
        return static_cast<uint8_t>(10 + (c - 'a'));
    }
    if (c >= 'A' && c <= 'Z') {
        return static_cast<uint8_t>(10 + (c - 'A'));
    }
    return 0;
}
