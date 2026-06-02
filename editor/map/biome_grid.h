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

// Dijkstra multi-fuente: asigna cada celda al bioma que la alcanza con menor
// costo. Los biomas más grandes expanden más rápido (velocidad = sqrt(area)).
// Devuelve, por celda (row-major, idx = y * width + x), el índice del bioma
// dueño dentro de `sources`, o -1 si ninguna fuente la alcanza.
// El orden de `sources` define el desempate entre biomas a igual costo.
std::vector<int> computeBiomeOwners(int width, int height,
                                    const std::vector<BiomeSource>& sources);

#endif
