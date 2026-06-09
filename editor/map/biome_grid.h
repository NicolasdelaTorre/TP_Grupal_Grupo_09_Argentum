#ifndef ARGENTUM_EDITOR_MAP_BIOME_GRID_H
#define ARGENTUM_EDITOR_MAP_BIOME_GRID_H

#include <vector>

// Rectángulo fuente de un bioma, en celdas.
struct BiomeSource {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

// asigna a cada celda el bioma que la alcanza con menor costo (Dijkstra multi-fuente).
std::vector<int> computeBiomeOwners(int width, int height, const std::vector<BiomeSource>& sources);

// Marca las celdas exteriores de un environment (flood fill).
std::vector<bool> computeExteriorCells(int width, int height, const std::vector<bool>& is_wall);

#endif
