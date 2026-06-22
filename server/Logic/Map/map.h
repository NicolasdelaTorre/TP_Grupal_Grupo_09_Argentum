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

// Obstáculo colocado en el mapa (solo para que el cliente lo dibuje). El tipo
// lógico (ObstacleCode) vive en la celda (Cell::obstacleId); el render se
// resuelve por la textura, así que acá no hace falta.
struct PlacedObstacle {
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
    // Ruta del sprite relativa a common/assets/images/subcarpeta
    std::string texture;
    // 0 = abajo-izq, 1 = abajo-centro, 2 = abajo-der.
    uint8_t texture_anchor = 0;
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
    std::vector<Cell> cells;  // obstáculos, paredes y salidas acá
    std::vector<CreatureSpawn> spawns;
    std::vector<Position> exits;
    std::vector<PlacedObstacle> obstacles;  // obstáculos del environment para el render
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

// NPC amigo (merchant/banker/priest) cargado de las ciudades del YAML.
// Tiene id propio para que el cliente pueda seleccionarlo con click.
struct FriendlyNpc {
    uint16_t id;
    int16_t x;
    int16_t y;
    uint8_t type;  // wire byte de NpcCode: MERCHANT=6, BANKER=7, PRIEST=8
    std::string name;
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
    uint16_t friendlyIdCounter = 10000;  // ids de amigos ≥ 10000 para no chocar con hostiles
    uint16_t width;
    uint16_t height;
    std::vector<Cell> cells;
    Position spawn;
    std::vector<LoadedEntry> entries;
    std::vector<Biome> biomes;
    std::vector<PlacedObstacle> obstacles;  // obstáculos del overworld
    std::vector<FriendlyNpc> friendlyNpcs;
    std::unordered_map<uint16_t, std::unique_ptr<NPC>> npcs;

    void setNPC();

    void spawnNPC(const Biome& biome, std::vector<Cell>& cells, uint8_t mapId);

public:
    // Constructor con celdas ya armadas (lo usa el YAML loader).
    Map(uint16_t width, uint16_t height, std::vector<Cell> cells, Position spawn, std::vector<LoadedEntry> entries, std::vector<Biome> biomes, std::vector<PlacedObstacle> obstacles = {});

    uint16_t getWidth(uint8_t mapId) const;

    uint16_t getHeight(uint8_t mapId) const;

    uint16_t getCellCount(uint8_t mapId) const;

    Cell getCell(size_t index, uint8_t mapId) const;

    // sirve para overworld y environments
    const std::vector<PlacedObstacle>& getObstacles(uint8_t mapId) const;

    Position getPlayerSpawn(uint8_t mapId);

    std::vector<uint16_t> getAllNPCIds() const;

    Creature* getNPC(uint16_t npcId);

    std::string getMapId(uint16_t x, uint16_t y);

    Position getEntryPosition(uint8_t mapId);

    Position getEntrySpawnPosition(const std::string& mapId);

    // Devuelve true si (x, y) está dentro de los límites del mapa.
    bool isInBounds(int16_t x, int16_t y, uint8_t mapId) const;

    // Devuelve true si (x, y) está en bounds y es transitable.
    bool isWalkable(int16_t x, int16_t y, uint8_t mapId) const;

    // Devuelve true si (x, y) está en bounds y la celda es zona segura
    bool isSafeZone(int16_t x, int16_t y, uint8_t mapId) const;

    bool occupiedByEntity(int16_t x, int16_t y, uint8_t mapId) const;

    uint16_t nextEntity(int16_t x, int16_t y, bool isPlayer, uint8_t mapId, int16_t targetId);

    // Mueve el entityId de (oldX, oldY) a (newX, newY) actualizando ambas celdas.
    bool moveEntity(int entityId, int16_t oldX, int16_t oldY, int16_t newX, int16_t newY, bool isPlayer, uint8_t mapId);

    // Limpia el entityId de la celda. Se llama al desconectar / morir.
    void removeEntity(int16_t x, int16_t y, uint8_t mapId, bool isPlayer);

    uint16_t entityInDistance(int16_t x, int16_t y, bool isPlayer, uint8_t mapId, int16_t targetId);

    Position placeEntity(int entityId, int16_t x, int16_t y, bool isPlayer, uint8_t mapId);

    Position searchPlayer(int16_t x, int16_t y, uint8_t mapId, std::string biomeType);

    bool checkNPCAlive(uint16_t npcId);

    bool checkIfNPCIsNextToAPlayer(uint16_t npcId, uint8_t mapId);

    bool isACreature(uint16_t npcId);

    bool checkIfThePositionHasAnEntry(int16_t x, int16_t y, uint8_t mapId);

    Position placePlayerIntoTheDungeon(int playerId, Position playerPosition, const std::string& mapId);

    Position placePlayerIntoTheOverworld(int playerId, uint8_t mapId);

    // ── NPCs amigos (merchant/banker/priest) ────────────────────────────
    // Registra un amigo en el mapa con id auto-incremental ≥ 10000.
    // Devuelve el id asignado.
    uint16_t addFriendlyNpc(int16_t x, int16_t y, uint8_t type, std::string name);

    // Snapshot de los amigos cargados (para mandar NewNpcEvent al loguearse).
    const std::vector<FriendlyNpc>& getFriendlyNpcs() const;

    // Devuelve nullptr si no existe ese id.
    const FriendlyNpc* getFriendlyNpc(uint16_t id) const;

    // Distancia del jugador al amigo. -1 si no existe el amigo.
    int friendlyNpcDistance(int16_t playerX, int16_t playerY, uint16_t friendlyId) const;
    int calculateTeleportingTime(Position playerPosition, uint8_t mapId);

    Position searchNearestPriest(int16_t x, int16_t y);

    Position getRandomPosition(std::string biomeType, uint8_t mapId);

    bool positionInBiome(Position pos, std::string biomeType);
};

#endif
