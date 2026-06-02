#include "map.h"

#include <stdexcept>
#include <utility>

Map::Map(uint16_t width, uint16_t height): width(width), height(height) {
    cells.resize(static_cast<size_t>(width) * height);
    initializeMap();
}

Map::Map(uint16_t width, uint16_t height, std::vector<Cell> cells):
        width(width), height(height), cells(std::move(cells)) {
    if (this->cells.size() != static_cast<size_t>(width) * height) {
        throw std::invalid_argument("Map Error: cells vector size does not match width * height");
    }
}

void Map::initializeMap() {
    for (auto& cell: cells) {
        cell.textureId = 0;
        cell.obstacleId = 0;
        cell.playerId = 0;
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
    return x >= 0 && y >= 0 && x < static_cast<int16_t>(width) && y < static_cast<int16_t>(height);
}

bool Map::isWalkable(int16_t x, int16_t y) const {
    if (!isInBounds(x, y))
        return false;
    return cells[static_cast<size_t>(y) * width + x].isWalkable;
}

bool Map::occupiedByPlayer(int16_t x, int16_t y) const {
    if (!isInBounds(x, y))
        return false;
    return cells[static_cast<size_t>(y) * width + x].playerId != 0;
}

uint8_t Map::isEntityInSight(int16_t x, int16_t y, const std::string& direction,
                             bool distanceWeapon) {
    size_t iterations = distanceWeapon ? 5 : 1;
    for (size_t i = 0; i < iterations; i++) {
        if (direction == "top") {
            y -= 1;
        } else if (direction == "bottom") {
            y += 1;
        } else if (direction == "left") {
            x -= 1;
        } else if (direction == "right") {
            x += 1;
        } else {
            throw std::invalid_argument("Map Error: invalid direction");
        }

        if (!isInBounds(x, y)) {
            return 0;  // out of bounds
        }

        Cell cell = getCell(static_cast<size_t>(y) * width + x);
        if (cell.playerId != 0) {
            return cell.playerId;  // player in sight
        }
    }

    return 0;  // no player in sight
}

void Map::placePlayer(int playerId, int16_t x, int16_t y) {
    if (!isInBounds(x, y)) {
        throw std::out_of_range("Map Error: trying to place player out of bounds");
    }

    // Preguntar a Martín sobre si el juego esta full cargado de jugadores
    while (occupiedByPlayer(x, y)) {
        // If the cell is already occupied by another player, look for the next free cell.
        x = (x + 1) % width;
        if (x == 0) {
            y = (y + 1) % height;
        }
    }

    cells[static_cast<size_t>(y) * width + x].playerId = static_cast<uint8_t>(playerId);
}
