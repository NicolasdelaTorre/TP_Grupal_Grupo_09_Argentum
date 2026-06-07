#ifndef MAP_H
#define MAP_H

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

#include <string>

#include "../../common/position.h"

#include "NPC/npc.h"
#include "NPC/creature.h"

// Tile del mapa (datos estáticos).
struct Cell {
    uint16_t textureId;
    uint16_t obstacleId;  // 0 si no hay obstáculo
    uint8_t playerId;
    uint16_t npcId;
    bool isWalkable;
    bool safeZone;
};

// Spawn de criaturas: qué criatura y cuántas como máximo.
struct CreatureSpawn {
    std::string creature;
    uint16_t maxPopulation = 0;
};

// Environment asociado a una entrada del mapa principal.
struct LoadedEnvironment {
    std::string id;
    std::string name;
    std::string type;
    int16_t width = 0;
    int16_t height = 0;
    Position playerSpawn;
    std::vector<Cell> cells;  // obstáculos y paredes acá
    std::vector<CreatureSpawn> spawns;
    std::string floorColor;
};

// Entrada con el environment al que lleva.
struct LoadedEntry {
    std::string id;
    std::string type;
    LoadedEnvironment environment;
    int16_t x = 0;
    int16_t y = 0;
    int16_t width = 1;
    int16_t height = 1;
};

// Bioma cargado desde el editor (zona de tipo "biome"): tipo (el `template` de
// la zona en el YAML), posición/tamaño del área que ocupa y sus spawns.
struct Biome {
    std::string type;  // template del bioma (ej. "cementerio")
    Position position;  // esquina sup izquierda del área
    int16_t width = 0;
    int16_t height = 0;
    std::vector<CreatureSpawn> spawns;
};

// Mapa estático: no cambia una vez cargado. Los jugadores los maneja el Game.
class Map {
private:
    uint16_t npcIdCounter;
    uint16_t width;
    uint16_t height;
    std::vector<Cell> cells;
    Position spawn;
    std::vector<LoadedEntry> entries;
    std::vector<Biome> biomes;
    std::unordered_map<uint16_t, std::unique_ptr<NPC>> npcs;

    void setNPC();

    void spawnNPC(const Biome& biome);

public:
    // Constructor con celdas ya armadas (lo usa el YAML loader).
    Map(uint16_t width, uint16_t height, std::vector<Cell> cells, Position spawn, std::vector<LoadedEntry> entries, std::vector<Biome> biomes);

    uint16_t getWidth() const;

    uint16_t getHeight() const;

    uint16_t getCellCount() const;

    Cell getCell(size_t index) const;

    Position getPlayerSpawn();

    std::vector<uint16_t> getAllNPCIds() const;

    Creature* getNPC(uint16_t npcId);

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

    bool checkNPCAlive(uint16_t npcId);

    bool checkIfNPCIsNextToAPlayer(uint16_t npcId);

    bool isACreature(uint16_t npcId);
};

#endif
