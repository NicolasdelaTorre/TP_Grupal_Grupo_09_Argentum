#pragma once

#include <cstdint>

#include "../common/DTOs.h"

// Mapping textureId → TileCode del renderer.
//
// TODO(arquitectura): que el server mande una tabla de biomas junto al MAP
// (índice → nombre/textura) para hacerlo robusto. Hoy hardcodeamos según el
// orden del mapa actual (`server/assets/maps/mapa_completo.yaml`).
inline TileCode tileTypeFromTextureId(uint16_t textureId) {
    switch (textureId) {
        case 1:
            return TileCode::GRASS;
        case 2:
            return TileCode::WATER;
        case 3:
            return TileCode::DIRT;
        case 4:
            return TileCode::SAND;
        case 5:
            return TileCode::INTERIOR;
        default:
            return TileCode::GRASS;
    }
}
