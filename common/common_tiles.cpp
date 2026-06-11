#include "common_tiles.h"

namespace {

const std::vector<FloorTile> FLOOR_TILES = {
        {1, "explanada", "tiles/explanada.png", "#A8C97F"},
        {2, "bosque_oscuro", "tiles/bosque_oscuro.png", "#2E4A2E"},
        {3, "bosque_aranas", "tiles/bosque.png", "#5B8C5A"},
        {4, "desierto", "tiles/desierto.png", "#D6D6D6"},
        {5, "cementerio", "tiles/cementerio.png", "#7D8C8C"},
        {6, "nieve", "tiles/nieve.png", "#D6E8F2"},
        {7, "rocosas", "tiles/rocosa.png", "#888B8D"},
        {8, "pantano_embrujado", "tiles/pantano.png", "#6B8E23"},
        {9, "adoquin", "tiles/adoquin.png", "#9A9A9A"},
        {10, "adoquin_oscuro", "tiles/adoquin_oscuro.png", "#5C5C5C"},
        {11, "madera", "tiles/madera.png", "#7A4B25"},
        {12, "cueva", "tiles/cueva.png", "#3A2A20"},
        {13, "mazmorra", "tiles/mazmorra.png", "#2A2A2A"},
        {EXTERIOR_TILE_VALUE, "exterior", "tiles/black.png", "#000000"},
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

const FloorTile* floor_tile_from_texture(const std::string& texture) {
    if (texture.empty()) {
        return nullptr;
    }
    for (const auto& tile: FLOOR_TILES) {
        if (texture == tile.texture) {
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
