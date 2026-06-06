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
        cell.npcId = 0;
        cell.isWalkable = true;
        cell.safeZone = false;
        cell.ocuppiedByMerchant = false;
        cell.ocuppiedByBanker = false;
    }

    /*
    for (auto& biome: biomes) {
        biome.typeBiome = 0;
        biome.size = 0;
        biome.creaturesCount = 0;
    }
        */
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

bool Map::occupiedByEntity(int16_t x, int16_t y) const {
    if (!isInBounds(x, y))
        return false;
    return cells[static_cast<size_t>(y) * width + x].playerId != 0 ||
           cells[static_cast<size_t>(y) * width + x].npcId != 0;
}

uint8_t Map::nextEntity(int16_t x, int16_t y, bool isPlayer) {
    for (size_t i = 0; i < 4; i++) {
        // Check position in the current direction
        switch (i) {
            case 0:
                y -= 1;
                break;  // Up
            case 1:
                y += 1;
                break;  // Down
            case 2:
                x -= 1;
                break;  // Left
            case 3:
                x += 1;
                break;  // Right
        }

        if (!isInBounds(x, y)) {
            continue;  // out of bounds
        }

        Cell cell = getCell(static_cast<size_t>(y) * width + x);
        if (isPlayer && cell.playerId != 0) {
            return cell.playerId;  // player in sight
        }

        if (!isPlayer && cell.npcId != 0) {
            return cell.npcId;  // npc in sight
        }
    }

    return 0;  // no entity in sight
}

uint8_t Map::entityInDistance(int16_t x, int16_t y, bool isPlayer) {
    for (int16_t dy = -3; dy <= 3; ++dy) {
        for (int16_t dx = -3; dx <= 3; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            int16_t checkX = x + dx;
            int16_t checkY = y + dy;
            if (!isInBounds(checkX, checkY)) {
                continue;
            }

            Cell cell = getCell(static_cast<size_t>(checkY) * width + checkX);
            if (isPlayer && cell.playerId != 0) {
                return cell.playerId;  // player in distance
            }

            if (!isPlayer && cell.npcId != 0) {
                return cell.npcId;  // npc in distance
            }
        }
    }

    return 0;  // no entity in distance
}

void Map::placeEntity(int entityId, int16_t x, int16_t y, bool isPlayer) {
    if (!isInBounds(x, y)) {
        throw std::out_of_range("Map Error: trying to place player/NPC out of bounds");
    }

    while (occupiedByEntity(x, y)) {
        // If the cell is already occupied by another player, look for the next free cell.
        x = (x + 1) % width;
        if (x == 0) {
            y = (y + 1) % height;
        }
    }

    if (isPlayer) {
        cells[static_cast<size_t>(y) * width + x].playerId = static_cast<uint8_t>(entityId);
    } else {
        cells[static_cast<size_t>(y) * width + x].npcId = static_cast<uint8_t>(entityId);
    }
}

Position Map::searchPlayer(int16_t x, int16_t y) {
    for (int16_t dy = -3; dy <= 3; ++dy) {
        for (int16_t dx = -3; dx <= 3; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            int16_t checkX = x + dx;
            int16_t checkY = y + dy;
            if (!isInBounds(checkX, checkY)) {
                continue;
            }

            Cell cell = getCell(static_cast<size_t>(checkY) * width + checkX);
            if (cell.playerId != 0) {
                return Position{checkX, checkY};
            }
        }
    }

    return Position{-1, -1};
}

void Map::movePlayer(int playerId, int16_t oldX, int16_t oldY, int16_t newX, int16_t newY) {
    if (isInBounds(oldX, oldY)) {
        cells[static_cast<size_t>(oldY) * width + oldX].playerId = 0;
    }
    if (isInBounds(newX, newY)) {
        cells[static_cast<size_t>(newY) * width + newX].playerId = static_cast<uint8_t>(playerId);
    }
}

void Map::removePlayer(int16_t x, int16_t y) {
    if (isInBounds(x, y)) {
        cells[static_cast<size_t>(y) * width + x].playerId = 0;
    }
}
