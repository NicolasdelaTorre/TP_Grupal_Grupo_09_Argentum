#include "map.h"

#include <algorithm>

Map::Map(uint16_t width, uint16_t height): width(width), height(height) {
    cells.resize(width * height);
    initializeMap();
}

void Map::initializeMap() {
    for (auto& cell: cells) {
        cell.isWalkable = true;
        cell.occupiedByPlayer = false;
    }
}

void Map::addPlayer() {
    for (;;) {
        auto itCellFree = std::find_if(cells.begin(), cells.end(), [](const Cell& cell) {
            return cell.isWalkable && !cell.occupiedByPlayer;
        });

        if (itCellFree != cells.end()) {
            itCellFree->occupiedByPlayer = true;
            return;
        }
    }
}

bool Map::movePlayer(const std::string& direction, int16_t x, int16_t y) {
    int16_t newX = x;
    int16_t newY = y;

    if (direction == "top") {
        newY -= 1;
    } else if (direction == "bottom") {
        newY += 1;
    } else if (direction == "left") {
        newX -= 1;
    } else if (direction == "right") {
        newX += 1;
    } else {
        // Invalid direction
        return false;
    }

    if (newX < 0 || newX >= width || newY < 0 || newY >= height) {
        // Out of map bounds
        return false;
    }

    size_t newPosition = newY * width + newX;
    size_t currentPosition = y * width + x;

    if (!cells[newPosition].isWalkable || cells[newPosition].occupiedByPlayer) {
        // Ocuppied cell
        return false;
    }

    // Free current cell
    cells[currentPosition].occupiedByPlayer = false;
    // Occupy the new cell
    cells[newPosition].occupiedByPlayer = true;

    // Movement successful
    return true;
}
