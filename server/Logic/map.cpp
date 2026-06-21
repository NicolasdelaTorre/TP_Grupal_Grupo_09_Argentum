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
#include "../../common/game_constants.h"
#include <iostream>

Map::Map(uint16_t width, uint16_t height, std::vector<Cell> cells, Position spawn, std::vector<LoadedEntry> entries, std::vector<Biome> biomes, std::vector<PlacedObstacle> obstacles):
        npcIdCounter(1), width(width), height(height), cells(std::move(cells)), spawn(spawn), entries(std::move(entries)), biomes(std::move(biomes)), obstacles(std::move(obstacles)) {
    if (this->cells.size() != static_cast<size_t>(width) * height) {
        throw std::invalid_argument("Map Error: cells vector size does not match width * height");
    }

    setNPC();

    for (auto& cell : this->cells) {
        if (cell.obstacleId == static_cast<uint8_t>(ObstacleCode::ENTRY)) {
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
            case static_cast<uint8_t>(ObstacleCode::NPC_BANKER):
                npcType = "banker";
                break;
            case static_cast<uint8_t>(ObstacleCode::NPC_MERCHANT):
                npcType = "trader";
                break;
            case static_cast<uint8_t>(ObstacleCode::NPC_PRIEST):
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

        if (npcType == "")
            std::cout << "El error es en el NPC numero: " << newNpcId << std::endl;
        else 
            std::cout << "NPC " << npcType << " spawned at (" << x << ", " << y << ") with ID " << newNpcId << std::endl;
    }

    // Aggresive NPCs
    // Overworld
    for (const auto& biome : biomes) {
        spawnNPC(biome, cells, 0);
    }

    // Dungeons
    for (auto& entry : entries) {
        std::cout << "Entry: (" << entry.x << ", " << entry.y << ")" << std::endl;
        Biome biome;
        biome.type = entry.environment.type;
        biome.position = {0, 0};
        biome.width = entry.environment.width;
        biome.height = entry.environment.height;
        biome.spawns = entry.environment.spawns;

        spawnNPC(biome, entry.environment.cells, static_cast<uint8_t>(entry.id[entry.id.size() - 1] - '0'));
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

            uint16_t newNpcId = npcIdCounter;
            npcIdCounter++;

            // Ocuppy cell
            Cell& targetCell = cells[static_cast<size_t>(pos.y) * getWidth(mapId) + pos.x];
            targetCell.npcId = newNpcId;

            // Save npc
            npcs[newNpcId] = std::make_unique<Creature>(newNpcId, spawnInfo.creature, mapId, pos.x, pos.y, biome.type);

            if (spawnInfo.creature == "")
                std::cout << "El error es en el NPC numero: " << newNpcId << std::endl;
        }
    }
}

uint16_t Map::getWidth(uint8_t mapId) const { 
    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                return static_cast<uint16_t>(entry.environment.width);
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
                return static_cast<uint16_t>(entry.environment.height);
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
                return static_cast<uint16_t>(entry.environment.width * entry.environment.height);
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

const std::vector<PlacedObstacle>& Map::getObstacles(uint8_t mapId) const {
    if (mapId > 0) {
        for (const auto& entry: entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                return entry.environment.obstacles;
            }
        }

        throw std::runtime_error("Map Error: entry not found for mapId " + std::to_string(mapId));
    }

    return obstacles;
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
        if (x >= entry.x && y >= entry.y && x < entry.x + entry.width &&
            y < entry.y + entry.height) {
            return entry.id;
        }
    }

    return "";
}

Position Map::getEntryPosition(uint8_t mapId) {
    for (const auto& entry : entries) {
        if (entry.id[entry.id.size() - 1] == '0' + mapId) {
            return {entry.x, entry.y};
        }
    }

    throw std::runtime_error("Map Error: entry not found for mapId " + std::to_string(mapId));
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
                return x >= 0 && y >= 0 && x < static_cast<int16_t>(entry.environment.width) &&
                       y < static_cast<int16_t>(entry.environment.height);
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
                return entry.environment.cells[static_cast<size_t>(y) * entry.environment.width + x].isWalkable;
            }
        }

        throw std::runtime_error("Map Error: entry not found for mapId " + std::to_string(mapId));
    }

    return cells[static_cast<size_t>(y) * width + x].isWalkable;
}

bool Map::isSafeZone(int16_t x, int16_t y, uint8_t mapId) const {
    if (!isInBounds(x, y, mapId)) return false;
    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                return entry.environment.cells[static_cast<size_t>(y) * entry.environment.width + x].safeZone;
            }
        }
        return false;
    }
    return cells[static_cast<size_t>(y) * width + x].safeZone;
}

bool Map::occupiedByEntity(int16_t x, int16_t y, uint8_t mapId) const {
    if (!isInBounds(x, y, mapId))
        return false;

    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                return entry.environment.cells[static_cast<size_t>(y) * entry.environment.width + x].playerId != 0 ||
                       entry.environment.cells[static_cast<size_t>(y) * entry.environment.width + x].npcId != 0;
            }
        }

        throw std::runtime_error("Map Error: entry not found for mapId " + std::to_string(mapId));
    }

    return cells[static_cast<size_t>(y) * width + x].playerId != 0 ||
           cells[static_cast<size_t>(y) * width + x].npcId != 0;
}

uint16_t Map::nextEntity(int16_t x, int16_t y, bool isPlayer, uint8_t mapId, int16_t targetId) {
    // All possible directions (up, down, left, right, and diagonals)
    const int16_t dx[] = {  0,   0,  -1,   1,      -1,       1,      -1,       1 };
    const int16_t dy[] = { -1,   1,   0,   0,      -1,      -1,       1,       1 };

    for (size_t i = 0; i < 8; i++) {
        // Check position in the current direction
        int16_t checkX = x + dx[i];
        int16_t checkY = y + dy[i];

        if (!isInBounds(checkX, checkY, mapId)) {
            continue;  // out of bounds
        }

        uint16_t currentWidth = getWidth(mapId);

        Cell cell = getCell(static_cast<size_t>(checkY) * currentWidth + checkX, mapId);

        if (isPlayer && cell.playerId == targetId) {
            return cell.playerId;  // player in sight
        }

        if (!isPlayer && (cell.npcId == targetId || (targetId == -1 && cell.npcId != 0))) {
            return cell.npcId;  // npc in sight
        }
    }

    return 0;  // no entity in sight
}

uint16_t Map::entityInDistance(int16_t x, int16_t y, bool isPlayer, uint8_t mapId, int16_t targetId) {
    for (int16_t dy = -COMBAT_RANGE; dy <= COMBAT_RANGE; ++dy) {
        for (int16_t dx = -COMBAT_RANGE; dx <= COMBAT_RANGE; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            int16_t checkX = x + dx;
            int16_t checkY = y + dy;
            if (!isInBounds(checkX, checkY, mapId)) {
                continue;
            }

            uint16_t currentWidth = getWidth(mapId);

            Cell cell = getCell(static_cast<size_t>(checkY) * currentWidth + checkX, mapId);
            if (isPlayer && cell.playerId == targetId) {
                return cell.playerId;  // player in distance
            }

            if (!isPlayer && (cell.npcId == targetId || (targetId == -1 && cell.npcId != 0))) {
                return cell.npcId;  // npc in distance
            }
        }
    }

    return 0;  // no entity in distance
}

Position Map::placeEntity(int entityId, int16_t x, int16_t y, bool isPlayer, uint8_t mapId) {
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

    std::vector<Cell>* cells = &this->cells;
    if (mapId > 0) {
        for (auto& entry : entries) {
            if (entry.id.back() == '0' + mapId) {
                cells = &entry.environment.cells;
                break;
            }
        }
    }

    if (isPlayer) {
        (*cells)[static_cast<size_t>(y) * width + x].playerId = static_cast<uint16_t>(entityId);
    } else {
        (*cells)[static_cast<size_t>(y) * width + x].npcId = static_cast<uint16_t>(entityId);
    }

    return Position{x, y};
}

Position Map::searchPlayer(int16_t x, int16_t y, uint8_t mapId, std::string biomeType) {
    for (int16_t dy = -COMBAT_RANGE; dy <= COMBAT_RANGE; ++dy) {
        for (int16_t dx = -COMBAT_RANGE; dx <= COMBAT_RANGE; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            int16_t checkX = x + dx;
            int16_t checkY = y + dy;
            if (!isInBounds(checkX, checkY, mapId)) {
                continue;
            }

            uint16_t currentWidth = getWidth(mapId);

            Cell cell = getCell(static_cast<size_t>(checkY) * currentWidth + checkX, mapId);
            // En el overworld las criaturas solo persiguen dentro de su bioma; en
            // las mazmorras (mapId > 0) no hay biomas, así que persiguen sin filtro.
            if (cell.playerId != 0 &&
                (mapId > 0 || positionInBiome({checkX, checkY}, biomeType))) {
                return Position{checkX, checkY};
            }
        }
    }

    return Position{-1, -1};
}

bool Map::moveEntity(int entityId, int16_t oldX, int16_t oldY, int16_t newX, int16_t newY, bool isPlayer, uint8_t mapId) {
    std::vector<Cell>* cells = &this->cells;
    if (mapId > 0) {
        for (auto& entry : entries) {
            if (entry.id.back() == '0' + mapId) {
                cells = &entry.environment.cells;
                break;
            }
        }
    }

    uint16_t currentWidth = getWidth(mapId);

    if (isInBounds(oldX, oldY, mapId) && isInBounds(newX, newY, mapId) && !occupiedByEntity(newX, newY, mapId) && isWalkable(newX, newY, mapId)) {
        if (isPlayer) {
            (*cells)[static_cast<size_t>(oldY) * currentWidth + oldX].playerId = 0;
            (*cells)[static_cast<size_t>(newY) * currentWidth + newX].playerId = static_cast<uint16_t>(entityId);
        } else {
            (*cells)[static_cast<size_t>(oldY) * currentWidth + oldX].npcId = 0;
            (*cells)[static_cast<size_t>(newY) * currentWidth + newX].npcId = static_cast<uint16_t>(entityId);
        }
        return true;
    }

    return false;
}

void Map::removeEntity(int16_t x, int16_t y, uint8_t mapId, bool isPlayer) {
    if (isInBounds(x, y, mapId)) {
        std::vector<Cell>* cells = &this->cells;
        if (mapId > 0) {
            for (auto& entry: entries) {
                if (entry.id.back() == '0' + mapId) {
                    cells = &entry.environment.cells;
                    break;
                }
            }
        }

        if (isPlayer) {
            (*cells)[static_cast<size_t>(y) * getWidth(mapId) + x].playerId = 0;
        } else {
            (*cells)[static_cast<size_t>(y) * getWidth(mapId) + x].npcId = 0;
        }
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
            return nextEntity(pos.x, pos.y, true, mapId, -1) != 0;  // Check if there's a player next to the NPC
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

    // If the player is in a dungeon
    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                for (const auto& exit : entry.environment.exits) {
                    if (exit.x == x && exit.y == y) {
                        return true;
                    }
                }
            }
        }

        return false;  // No exit at the position
    }

    // If the player in in the overworld
    for (const auto& entry : entries) {
        if (x >= entry.x && y >= entry.y && x < entry.x + entry.width &&
            y < entry.y + entry.height) {
            return true;
        }
    }

    return false;  // No entry at the position
}

Position Map::placePlayerIntoTheDungeon(int playerId, Position playerPosition, const std::string& mapId) {
    for (const auto& entry : entries) {
        if (entry.id == mapId) {
            removeEntity(playerPosition.x, playerPosition.y, 0, true);  // Remove player from overworld
            return placeEntity(playerId, entry.environment.playerSpawn.x, entry.environment.playerSpawn.y, true, mapId[mapId.size() - 1] - '0');
        }
    }

    throw std::runtime_error("Map Error: trying to place player into a non-existent dungeon with id " + mapId);
}

Position Map::placePlayerIntoTheOverworld(int playerId, uint8_t mapId) {
    for (const auto& entry: entries) {
        if (entry.id[entry.id.size() - 1] == '0' + mapId) {
            return placeEntity(playerId, entry.x, entry.y + 1, true, 0);
        }
    }

    throw std::runtime_error(
            "Map Error: trying to place player into the overworld from a non-existent dungeon "
            "with id "
            + std::to_string(mapId));
}

uint16_t Map::addFriendlyNpc(int16_t x, int16_t y, uint8_t type, std::string name) {
    uint16_t id = friendlyIdCounter++;
    friendlyNpcs.push_back(FriendlyNpc{id, x, y, type, std::move(name)});
    return id;
}

const std::vector<FriendlyNpc>& Map::getFriendlyNpcs() const { return friendlyNpcs; }

const FriendlyNpc* Map::getFriendlyNpc(uint16_t id) const {
    for (const auto& f: friendlyNpcs) {
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

int Map::calculateTeleportingTime(Position playerPosition, uint8_t mapId) {
    if (playerPosition.x == -1 || playerPosition.y == -1) {
        return -1;
    }

    int16_t x = playerPosition.x;
    int16_t y = playerPosition.y;

    // Check if the player is in a Dungeon
    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                x = entry.x;
                y = entry.y;
            }
        }
    }

    Position nearestPriestPos = searchNearestPriest(x, y);

    if (nearestPriestPos.x != -1 && nearestPriestPos.y != -1)
        return (std::abs(nearestPriestPos.x - x) + std::abs(nearestPriestPos.y - y)) * 1000;

    throw std::runtime_error("Map Error: no priest found");
}

Position Map::searchNearestPriest(int16_t x, int16_t y) {
    Position nearestPriestPos{-1, -1};
    int minDistance = std::numeric_limits<int>::max();

    for (const auto& npc : npcs) {
        if (npc.second->getName() == "priest") {
            Position npcPos = npc.second->getPosition();
            int distance = std::abs(npcPos.x - x) + std::abs(npcPos.y - y);
            if (distance < minDistance) {
                minDistance = distance;
                nearestPriestPos = npcPos;
            }
        }
    }

    if (nearestPriestPos.x != -1 && nearestPriestPos.y != -1) {
        return nearestPriestPos;
    }

    throw std::runtime_error("Map Error: no priest found");
}

Position Map::getRandomPosition(std::string biomeType, uint8_t mapId) {
    std::vector<Position> validCells;

    if (mapId > 0) {
        for (const auto& entry : entries) {
            if (entry.id[entry.id.size() - 1] == '0' + mapId) {
                for (int16_t y = 0; y < entry.environment.height; ++y) {
                    for (int16_t x = 0; x < entry.environment.width; ++x) {
                        Cell cell = entry.environment.cells[static_cast<size_t>(y) * entry.environment.width + x];
                        if (cell.isWalkable && 
                            !cell.safeZone && 
                            cell.playerId == 0 && 
                            cell.npcId == 0) {
                                validCells.push_back({x, y});
                        }
                    }
                }
            }
        }
    } else {
        for (const auto& biome : biomes) {
            if (biome.type == biomeType) {
                for (int16_t y = biome.position.y; y < biome.position.y + biome.height; ++y) {
                    for (int16_t x = biome.position.x; x < biome.position.x + biome.width; ++x) {
                        Cell& cell = cells[static_cast<size_t>(y) * width + x];
                        if (cell.isWalkable && 
                            !cell.safeZone && 
                            cell.playerId == 0 && 
                            cell.npcId == 0) {
                                validCells.push_back({x, y});
                        }
                    }
                }
            }
        }
    }

    if (validCells.empty()) {
        throw std::runtime_error("Map Error: no valid cells found for biome type " + biomeType);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, validCells.size() - 1);
    return validCells[dis(gen)];
}

bool Map::positionInBiome(Position pos, std::string biomeType) {
    for (const auto& biome : biomes) {
        if (biome.type == biomeType) {
            if (pos.x >= biome.position.x && pos.y >= biome.position.y &&
                pos.x < biome.position.x + biome.width && pos.y < biome.position.y + biome.height) {
                return true;
            }
        }
    }
    return false;
}
