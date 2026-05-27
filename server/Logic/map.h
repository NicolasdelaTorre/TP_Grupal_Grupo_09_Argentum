#ifndef MAP_H
#define MAP_H

#include <cstdint>
#include <string>
#include <vector>

struct Cell {
    uint16_t textureId;
    uint16_t obstacleId;  // 0 if there is no obstacle
    bool isWalkable;
    bool occupiedByPlayer;
    bool safeZone;
};

class Map {
private:
    uint16_t width;
    uint16_t height;
    std::vector<Cell> cells;

    void initializeMap();

public:
    Map(uint16_t width, uint16_t height);

    void addPlayer();

    bool movePlayer(const std::string& direction, int16_t x, int16_t y);

    uint16_t getWidth() const;

    uint16_t getHeight() const;

    uint16_t getCellCount() const;

    Cell getCell(size_t index) const;
};

#endif
