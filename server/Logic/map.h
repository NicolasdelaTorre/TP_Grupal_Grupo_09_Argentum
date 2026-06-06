#ifndef MAP_H
#define MAP_H

#include <cstdint>
#include <string>
#include <vector>

#include "../../common/position.h"

// Tile del mapa (datos estáticos).
struct Cell {
    uint16_t textureId;
    uint16_t obstacleId;  // 0 si no hay obstáculo
    uint8_t playerId;
    uint8_t npcId;
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

    bool occupiedByEntity(int16_t x, int16_t y) const;

    uint8_t nextEntity(int16_t x, int16_t y, bool isPlayer);

    // Mueve el playerId de (oldX, oldY) a (newX, newY) actualizando ambas celdas.
    void movePlayer(int playerId, int16_t oldX, int16_t oldY, int16_t newX, int16_t newY);

    // Limpia el playerId de la celda. Se llama al desconectar / morir.
    void removePlayer(int16_t x, int16_t y);

    uint8_t entityInDistance(int16_t x, int16_t y, bool isPlayer);

    void placeEntity(int entityId, int16_t x, int16_t y, bool isPlayer);

    Position searchPlayer(int16_t x, int16_t y);
};

#endif
