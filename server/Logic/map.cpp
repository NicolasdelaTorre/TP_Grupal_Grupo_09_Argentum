#include "map.h"

#include <stdexcept>

Map::Map(uint16_t width, uint16_t height): width(width), height(height) {
    cells.resize(static_cast<size_t>(width) * height);
    initializeMap();
}

void Map::initializeMap() {
    for (auto& cell: cells) {
        cell.textureId = 0;
        cell.obstacleId = 0;
        cell.isWalkable = true;
        cell.safeZone = false;
    }
}

uint16_t Map::getWidth() const { return width; }

uint16_t Map::getHeight() const { return height; }

uint16_t Map::getCellCount() const { return static_cast<uint16_t>(cells.size()); }

Cell Map::getCell(size_t index) const {
    if (index >= cells.size()) {
        throw std::out_of_range("Map Error: cell index out of range");
    }
    return cells[index];
}

bool Map::isInBounds(int16_t x, int16_t y) const {
    return x >= 0 && y >= 0 && x < static_cast<int16_t>(width) &&
           y < static_cast<int16_t>(height);
}

bool Map::isWalkable(int16_t x, int16_t y) const {
    if (!isInBounds(x, y))
        return false;
    return cells[static_cast<size_t>(y) * width + x].isWalkable;
}
