#ifndef YAML_MAP_LOADER_H
#define YAML_MAP_LOADER_H

#include <string>
#include <vector>

#include "../../common/position.h"

#include "map.h"

// Sirve tanto para obstáculos como para paredes; en las paredes, `type` guarda su template.
struct EnvironmentObstacle {
    std::string type;
    int16_t x = 0;
    int16_t y = 0;
    int16_t width = 1;
    int16_t height = 1;
};

// Environment asociado a una entrada del mapa principal.
struct LoadedEnvironment {
    std::string id;
    std::string name;
    std::string type;
    int16_t width = 0;
    int16_t height = 0;
    bool hasPlayerSpawn = false;
    Position playerSpawn{0, 0};
    std::vector<EnvironmentObstacle> obstacles;
    std::vector<EnvironmentObstacle> walls;
    std::string floorColor;
};

// Entrada (portal a una cueva). Se relaciona con un environment vía
// `environmentId` == LoadedEnvironment::id.
struct LoadedEntry {
    std::string id;
    std::string type;
    std::string environmentId;
    int16_t x = 0;
    int16_t y = 0;
    int16_t width = 1;
    int16_t height = 1;
};

// Map parseado del YAML + spawn point para los jugadores. `entries` y
// `environments` se parsean pero todavía no se usan en el juego.
struct LoadedMap {
    Map map;
    Position playerSpawn;
    std::vector<LoadedEntry> entries;
    std::vector<LoadedEnvironment> environments;
};

// Carga el mapa desde un archivo YAML. Tira runtime_error si falla.
LoadedMap loadMapFromYaml(const std::string& path);

#endif  // YAML_MAP_LOADER_H
