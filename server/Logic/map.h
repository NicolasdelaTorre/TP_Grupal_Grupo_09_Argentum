#ifndef MAP_H
#define MAP_H

#include <cstdint>
#include <vector>

// Tile del mapa (datos estáticos).
struct Cell {
    uint16_t textureId;
    uint16_t obstacleId;  // 0 si no hay obstáculo
    bool isWalkable;
    bool safeZone;
};

// Mapa estático: no cambia una vez cargado. Los jugadores los maneja el Game.
class Map {
private:
    uint16_t width;
    uint16_t height;
    std::vector<Cell> cells;

    void initializeMap();

public:
    Map(uint16_t width, uint16_t height);

    // Constructor con celdas ya armadas (lo usa el YAML loader).
    Map(uint16_t width, uint16_t height, std::vector<Cell> cells);

    uint16_t getWidth() const;
    uint16_t getHeight() const;
    uint16_t getCellCount() const;
    Cell getCell(size_t index) const;

    // Devuelve true si (x, y) está dentro de los límites del mapa.
    bool isInBounds(int16_t x, int16_t y) const;

    // Devuelve true si (x, y) está en bounds y es transitable.
    bool isWalkable(int16_t x, int16_t y) const;
};

#endif
