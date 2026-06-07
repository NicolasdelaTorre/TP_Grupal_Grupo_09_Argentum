#include "map.h"

#include <stdexcept>
#include <utility>
#include <random>
#include <algorithm>
#include <memory>

#include "NPC/creature.h"
#include "NPC/merchant.h"
#include "NPC/banker.h"
#include "../../common/DTOs.h"

Map::Map(uint16_t width, uint16_t height, std::vector<Cell> cells, Position spawn, std::vector<LoadedEntry> entries, std::vector<Biome> biomes):
        npcIdCounter(1), width(width), height(height), cells(std::move(cells)), spawn(spawn), entries(std::move(entries)), biomes(std::move(biomes)) {
    if (this->cells.size() != static_cast<size_t>(width) * height) {
        throw std::invalid_argument("Map Error: cells vector size does not match width * height");
    }

    setNPC();
}

void Map::setNPC() {
    // Friendly NPCs
    for (size_t i = 0; i < cells.size(); ++i) {
        Cell& cell = cells[i];
        std::string npcType;
        switch (cell.obstacleId) {
            case static_cast<uint8_t>(ObstacleType::NPC_BANKER):
                npcType = "banker";
                break;
            case static_cast<uint8_t>(ObstacleType::NPC_MERCHANT):
                npcType = "trader";
                break;
            case static_cast<uint8_t>(ObstacleType::NPC_PRIEST):
                npcType = "priest";
                break;
            default:
                // No NPC
                continue;
        }

        int16_t x = static_cast<int16_t>(i % width);
        int16_t y = static_cast<int16_t>(i / width);

        uint16_t newNpcId = npcIdCounter;
        npcIdCounter++;

        cell.npcId = newNpcId;
        
        if (npcType == "trader" || npcType == "priest")
            npcs[newNpcId] = std::make_unique<Merchant>(newNpcId, npcType, x, y);
        else if (npcType == "banker")
            npcs[newNpcId] = std::make_unique<Banker>(newNpcId, npcType, x, y);
    }

    // Aggresive NPCs
    // Overworld
    for (const auto& biome : biomes) {
        spawnNPC(biome);
    }

    // Dungeons
    // Proximamente
}

void Map::spawnNPC(const Biome& biome) {
    std::vector<Position> validCells;

    uint16_t startX = biome.position.x;
    int16_t startY = biome.position.y;
    int16_t endX = startX + biome.width;
    int16_t endY = startY + biome.height;

    for (int16_t y = startY; y < endY; ++y) {
        for (int16_t x = startX; x < endX; ++x) {
            if (!isInBounds(x, y)) continue;

            Cell& cell = cells[static_cast<size_t>(y) * width + x];

            if (cell.isWalkable && 
                !cell.safeZone && 
                cell.playerId == 0 && 
                cell.npcId == 0) {
                    // Save valid cell
                validCells.push_back({x, y});
            }
        }
    }

    // No valid positions
    if (validCells.empty()) return;

    // Radomize the valid cells
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(validCells.begin(), validCells.end(), gen);

    // Create the npcs
    size_t indexCell = 0;

    for (const auto& spawnInfo : biome.spawns) {
        for (uint16_t i = 0; i < spawnInfo.maxPopulation; ++i) {
            
            // No more valid cells
            if (indexCell >= validCells.size()) return; 

            Position pos = validCells[indexCell];
            indexCell++;

            uint16_t newNpcId = npcIdCounter++;

            // Ocuppy cell
            Cell& targetCell = cells[static_cast<size_t>(pos.y) * width + pos.x];
            targetCell.npcId = newNpcId;
            targetCell.isWalkable = false;

            // Save npc
            npcs[newNpcId] = std::make_unique<Creature>(newNpcId, spawnInfo.creature, id, pos.x, pos.y);
        }
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

Position Map::getPlayerSpawn() {
    return spawn;
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
