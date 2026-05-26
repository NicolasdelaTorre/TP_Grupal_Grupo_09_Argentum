#ifndef MAP_H
#define MAP_H

#include <cstdint>
#include <string>
#include <vector>

typedef struct Cell {
    bool isWalkable;
    bool occupiedByPlayer;
} Cell;

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
};

#endif
