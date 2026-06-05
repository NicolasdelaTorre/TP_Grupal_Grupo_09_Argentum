#pragma once

#include <cstdint>

#include "../common/DTOs.h"

// Mapping textureId → TileType del renderer.
//
// TODO(arquitectura): que el server mande una tabla de biomas junto al MAP
// (índice → nombre/textura) para hacerlo robusto. Hoy hardcodeamos según el
// orden del mapa actual (`server/assets/maps/mapa_completo.yaml`).
inline TileType tileTypeFromTextureId(uint16_t textureId) {
    switch (textureId) {
        case 1:
            return TileType::GRASS;
        case 2:
            return TileType::WATER;
        case 3:
            return TileType::DIRT;
        case 4:
            return TileType::SAND;
        case 5:
            return TileType::INTERIOR;
        default:
            return TileType::GRASS;
    }
}
