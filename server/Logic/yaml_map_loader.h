#ifndef YAML_MAP_LOADER_H
#define YAML_MAP_LOADER_H

#include <string>
#include <vector>

#include "../../common/common_biome.h"
#include "../../common/position.h"

#include "map.h"

// Environment asociado a una entrada del mapa principal.
struct LoadedEnvironment {
    std::string id;
    std::string name;
    std::string type;
    int16_t width = 0;
    int16_t height = 0;
    Position playerSpawn;
    std::vector<Cell> cells;  // obstáculos y paredes acá
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

// Spawn de criaturas asociado a un bioma: qué criatura y cuántas como máximo.
struct CreatureSpawn {
    std::string creature;
    uint16_t maxPopulation = 0;
};

// Bioma cargado desde el editor (zona de tipo "biome"): tipo, posición/tamaño
// del área que ocupa y los spawns de criaturas que tiene.
struct Biome {
    BiomeType type = BiomeType::NONE;
    Position position;  // esquina sup izquierda del área
    int16_t width = 0;
    int16_t height = 0;
    std::vector<CreatureSpawn> spawns;
};

// Map parseado del YAML + spawn point para los jugadores.
struct LoadedMap {
    Map map;
    Position playerSpawn;
    std::vector<LoadedEntry> entries;
    std::vector<LoadedEnvironment> environments;
    std::vector<Biome> biomes;
};

// Carga el mapa desde un archivo YAML. Tira runtime_error si falla.
LoadedMap loadMapFromYaml(const std::string& path);

#endif  // YAML_MAP_LOADER_H
