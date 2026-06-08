#include "map.h"

#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <random>
#include <algorithm>
#include <memory>

#include "NPC/creature.h"
#include "NPC/merchant.h"
#include "NPC/banker.h"
#include "../../common/DTOs.h"
#include <iostream>

Map::Map(uint16_t width, uint16_t height, std::vector<Cell> cells, Position spawn, std::vector<LoadedEntry> entries, std::vector<Biome> biomes):
        npcIdCounter(1), width(width), height(height), cells(std::move(cells)), spawn(spawn), entries(std::move(entries)), biomes(std::move(biomes)) {
    if (this->cells.size() != static_cast<size_t>(width) * height) {
        throw std::invalid_argument("Map Error: cells vector size does not match width * height");
    }

    setNPC();

    for (auto& cell : this->cells) {
        if (cell.obstacleId == static_cast<uint8_t>(ObstacleType::ENTRY)) {
            cell.isWalkable = true;
            cell.obstacleId = 0;
        }
    }
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
        spawnNPC(biome, cells, 0);
    }

    // Dungeons
    for (auto& entry : entries) {
        std::cout << "Entry: (" << entry.x << ", " << entry.y << ")" << std::endl;
        if (entry.type == "") {
            Biome biome;
            biome.type = entry.environment.type;
            biome.position = {entry.x, entry.y};
            biome.width = entry.width;
            biome.height = entry.height;
            biome.spawns = entry.environment.spawns;

            spawnNPC(biome, entry.environment.cells, static_cast<uint8_t>(entry.id[entry.id.size() - 1] - '0'));
        }
    }
}

void Map::spawnNPC(const Biome& biome, std::vector<Cell>& cells, uint8_t mapId) {
    std::vector<Position> validCells;

    uint16_t startX = biome.position.x;
    int16_t startY = biome.position.y;
    int16_t endX = startX + biome.width;
    int16_t endY = startY + biome.height;

    for (int16_t y = startY; y < endY; ++y) {
        for (int16_t x = startX; x < endX; ++x) {
            if (!isInBounds(x, y, mapId)) continue;

            Cell& cell = cells[static_cast<size_t>(y) * getWidth(mapId) + x];

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
            Cell& targetCell = cells[static_cast<size_t>(pos.y) * getWidth(mapId) + pos.x];
            targetCell.npcId = newNpcId;
            targetCell.isWalkable = false;

            // Save npc
            npcs[newNpcId] = std::make_unique<Creature>(newNpcId, spawnInfo.creature, 0, pos.x, pos.y);
        }
    }
}

uint16_t Map::getWidth(uint8_t mapId) const { 
    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                return static_cast<uint16_t>(entry.width);
            }
        }

        throw std::runtime_error("Map Error: entry not found for mapId " + std::to_string(mapId));
    }

    return width; 
}

uint16_t Map::getHeight(uint8_t mapId) const { 
    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                return static_cast<uint16_t>(entry.height);
            }
        }

        throw std::runtime_error("Map Error: entry not found for mapId " + std::to_string(mapId));
    }

    return height; 
}

uint16_t Map::getCellCount(uint8_t mapId) const { 
    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                return static_cast<uint16_t>(entry.width * entry.height);
            }
        }

        throw std::runtime_error("Map Error: entry not found for mapId " + std::to_string(mapId));
    }

    return static_cast<uint16_t>(cells.size());
}

Cell Map::getCell(size_t index, uint8_t mapId) const {
    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                if (index >= entry.environment.cells.size()) {
                    throw std::out_of_range("Map Error: cell index out of range for mapId " + std::to_string(mapId));
                }

                return entry.environment.cells[index];
            }
        }

        throw std::runtime_error("Map Error: entry not found for mapId " + std::to_string(mapId));
    }

    if (index >= cells.size()) {
        throw std::out_of_range("Map Error: cell index out of range");
    }
    return cells[index];
}

Position Map::getPlayerSpawn(uint8_t mapId) {
    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                return entry.environment.playerSpawn;
            }
        }

        throw std::runtime_error("Map Error: entry not found for mapId " + std::to_string(mapId));
    }

    return spawn;
}

std::vector<uint16_t> Map::getAllNPCIds() const {
    std::vector<uint16_t> npcIds;
    for (const auto& pair : npcs) {
        npcIds.push_back(pair.first);
    }
    return npcIds;
}

Creature* Map::getNPC(uint16_t npcId) {
    auto it = npcs.find(npcId);
    if (it != npcs.end()) {
        return dynamic_cast<Creature*>(it->second.get());
    }
    return nullptr;  // NPC not found or not a Creature
}

std::string Map::getMapId(uint16_t x, uint16_t y) {
    for (const auto& entry : entries) {
        if (entry.x == x && entry.y == y) {
            return entry.id;
        }
    }

    return "";
}

Position Map::getEntrySpawnPosition(const std::string& mapId) {
    for (const auto& entry : entries) {
        if (entry.id == mapId) {
            return entry.environment.playerSpawn;
        }
    }

    throw std::runtime_error("Map Error: entry not found for mapId " + mapId);
}

bool Map::isInBounds(int16_t x, int16_t y, uint8_t mapId) const {
    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                return x >= 0 && y >= 0 && x < static_cast<int16_t>(entry.width) && y < static_cast<int16_t>(entry.height);
            }
        }
    }

    return x >= 0 && y >= 0 && x < static_cast<int16_t>(width) && y < static_cast<int16_t>(height);
}

bool Map::isWalkable(int16_t x, int16_t y, uint8_t mapId) const {
    if (!isInBounds(x, y, mapId))
        return false;

    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                return entry.environment.cells[static_cast<size_t>(y) * entry.width + x].isWalkable;
            }
        }

        throw std::runtime_error("Map Error: entry not found for mapId " + std::to_string(mapId));
    }

    return cells[static_cast<size_t>(y) * width + x].isWalkable;
}

bool Map::occupiedByEntity(int16_t x, int16_t y, uint8_t mapId) const {
    if (!isInBounds(x, y, mapId))
        return false;

    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                return entry.environment.cells[static_cast<size_t>(y) * entry.width + x].playerId != 0 ||
                       entry.environment.cells[static_cast<size_t>(y) * entry.width + x].npcId != 0;
            }
        }

        throw std::runtime_error("Map Error: entry not found for mapId " + std::to_string(mapId));
    }

    return cells[static_cast<size_t>(y) * width + x].playerId != 0 ||
           cells[static_cast<size_t>(y) * width + x].npcId != 0;
}

uint8_t Map::nextEntity(int16_t x, int16_t y, bool isPlayer, uint8_t mapId) {
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

        if (!isInBounds(x, y, mapId)) {
            continue;  // out of bounds
        }

        Cell cell = getCell(static_cast<size_t>(y) * width + x, mapId);
        if (isPlayer && cell.playerId != 0) {
            return cell.playerId;  // player in sight
        }

        if (!isPlayer && cell.npcId != 0) {
            return cell.npcId;  // npc in sight
        }
    }

    return 0;  // no entity in sight
}

uint8_t Map::entityInDistance(int16_t x, int16_t y, bool isPlayer, uint8_t mapId) {
    for (int16_t dy = -3; dy <= 3; ++dy) {
        for (int16_t dx = -3; dx <= 3; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            int16_t checkX = x + dx;
            int16_t checkY = y + dy;
            if (!isInBounds(checkX, checkY, mapId)) {
                continue;
            }

            Cell cell = getCell(static_cast<size_t>(checkY) * width + checkX, mapId);
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

void Map::placeEntity(int entityId, int16_t x, int16_t y, bool isPlayer, uint8_t mapId) {
    if (!isInBounds(x, y, mapId)) {
        throw std::out_of_range("Map Error: trying to place player/NPC out of bounds");
    }

    uint16_t width = getWidth(mapId);
    uint16_t height = getHeight(mapId);

    while (occupiedByEntity(x, y, mapId)) {
        // If the cell is already occupied by another player, look for the next free cell.
        x = (x + 1) % width;
        if (x == 0) {
            y = (y + 1) % height;
        }
    }

    std::vector<Cell>& cells = this->cells;
    if (mapId > 0) {
        for (auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                cells = entry.environment.cells;
                break;
            }
        }
    }

    if (isPlayer) {
        cells[static_cast<size_t>(y) * width + x].playerId = static_cast<uint8_t>(entityId);
    } else {
        cells[static_cast<size_t>(y) * width + x].npcId = static_cast<uint8_t>(entityId);
    }
}

Position Map::searchPlayer(int16_t x, int16_t y, uint8_t mapId) {
    for (int16_t dy = -3; dy <= 3; ++dy) {
        for (int16_t dx = -3; dx <= 3; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            int16_t checkX = x + dx;
            int16_t checkY = y + dy;
            if (!isInBounds(checkX, checkY, mapId)) {
                continue;
            }

            Cell cell = getCell(static_cast<size_t>(checkY) * width + checkX, mapId);
            if (cell.playerId != 0) {
                return Position{checkX, checkY};
            }
        }
    }

    return Position{-1, -1};
}

void Map::moveEntity(int entityId, int16_t oldX, int16_t oldY, int16_t newX, int16_t newY, bool isPlayer, uint8_t mapId) {
    std::vector<Cell>& cells = this->cells;
    if (mapId > 0) {
        for (auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                cells = entry.environment.cells;
                break;
            }
        }
    }

    if (isInBounds(oldX, oldY, mapId)) {
        if (isPlayer) {
            cells[static_cast<size_t>(oldY) * width + oldX].playerId = 0;
        } else {
            cells[static_cast<size_t>(oldY) * width + oldX].npcId = 0;
        }
    }
    if (isInBounds(newX, newY, mapId)) {
        if (isPlayer) {
            cells[static_cast<size_t>(newY) * width + newX].playerId = static_cast<uint8_t>(entityId);
        } else {
            cells[static_cast<size_t>(newY) * width + newX].npcId = static_cast<uint8_t>(entityId);
        }
    }
}

void Map::removePlayer(int16_t x, int16_t y, uint8_t mapId) {
    if (isInBounds(x, y, mapId)) {
        cells[static_cast<size_t>(y) * getWidth(mapId) + x].playerId = 0;
    }
}

bool Map::checkNPCAlive(uint16_t npcId) {
    auto it = npcs.find(npcId);
     
    if (it == npcs.end()) {
        throw std::runtime_error("Map Error: NPC with id " + std::to_string(npcId) + " not found"); 
    }

    // Verify if the NPC is a creature
    Creature* creature = dynamic_cast<Creature*>(it->second.get());
    
    if (creature) {
        return !creature->isDead();
    }

    throw std::runtime_error("Map Error: NPC with id " + std::to_string(npcId) + " is not a creature");
}

bool Map::checkIfNPCIsNextToAPlayer(uint16_t npcId, uint8_t mapId) {
    auto it = npcs.find(npcId);
    if (it != npcs.end()) {
        Creature* creature = dynamic_cast<Creature*>(it->second.get());
        if (creature) {
            Position pos = creature->getPosition();
            return nextEntity(pos.x, pos.y, true, mapId) != 0;  // Check if there's a player next to the NPC
        }
    }
    return false;  // NPC not found or not a creature
}

bool Map::isACreature(uint16_t npcId) {
    auto it = npcs.find(npcId);
    if (it != npcs.end()) {
        return dynamic_cast<Creature*>(it->second.get()) != nullptr;
    }
    throw std::runtime_error("Map Error: NPC with id " + std::to_string(npcId) + " not found");
}

bool Map::checkIfThePositionHasAnEntry(int16_t x, int16_t y, uint8_t mapId) {
    if (!isInBounds(x, y, mapId)) {
        throw std::out_of_range("Map Error: trying to check entry out of bounds");
    }

    for (const auto& entry : entries) {
        if (entry.x == x && entry.y == y) {
            return true;
        }
    }

    return false;  // No entry at the position
}

void Map::placePlayerIntoTheDungeon(int playerId, const std::string& mapId) {
    for (const auto& entry : entries) {
        if (entry.id == mapId) {
            placeEntity(playerId, entry.environment.playerSpawn.x, entry.environment.playerSpawn.y, true, mapId[mapId.size() - 1] - '0');
            return;
        }
    }

    throw std::runtime_error("Map Error: trying to place player into a non-existent dungeon with id " + mapId);
}

uint16_t Map::addFriendlyNpc(int16_t x, int16_t y, uint8_t type, std::string name) {
    uint16_t id = friendlyIdCounter++;
    friendlyNpcs.push_back(FriendlyNpc{id, x, y, type, std::move(name)});
    return id;
}

const std::vector<FriendlyNpc>& Map::getFriendlyNpcs() const { return friendlyNpcs; }

const FriendlyNpc* Map::getFriendlyNpc(uint16_t id) const {
    for (const auto& f : friendlyNpcs) {
        if (f.id == id) return &f;
    }
    return nullptr;
}

int Map::friendlyNpcDistance(int16_t playerX, int16_t playerY, uint16_t friendlyId) const {
    const FriendlyNpc* f = getFriendlyNpc(friendlyId);
    if (!f) return -1;
    int dx = std::abs(static_cast<int>(playerX) - static_cast<int>(f->x));
    int dy = std::abs(static_cast<int>(playerY) - static_cast<int>(f->y));
    return std::max(dx, dy);
}
