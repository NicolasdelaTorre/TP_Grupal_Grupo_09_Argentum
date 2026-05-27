#ifndef MAP_H
#define MAP_H

#include <cstdint>
#include <vector>

// Información estática de un tile del mapa. No incluye estado dinámico
// (jugadores, items en el suelo, etc.) — eso lo maneja el Game.
struct Cell {
    uint16_t textureId;
    uint16_t obstacleId;  // 0 si no hay obstáculo
    bool isWalkable;
    bool safeZone;
};

// Mapa estático del mundo. Una vez construido, no muta: los jugadores y
// demás entidades viven en el Game, no acá. Esto evita la duplicación
// de estado y permite serializar el mapa una sola vez al iniciar.
class Map {
private:
    uint16_t width;
    uint16_t height;
    std::vector<Cell> cells;

    void initializeMap();

public:
    Map(uint16_t width, uint16_t height);

    uint16_t getWidth() const;
    uint16_t getHeight() const;
    uint16_t getCellCount() const;
    Cell getCell(size_t index) const;

    // Devuelve true si (x, y) está dentro de los límites del mapa.
    bool isInBounds(int16_t x, int16_t y) const;

    // Devuelve true si (x, y) está en bounds y es transitable.
    // No considera si hay un jugador parado ahí — eso lo evalúa el Game.
    bool isWalkable(int16_t x, int16_t y) const;
};

#endif
