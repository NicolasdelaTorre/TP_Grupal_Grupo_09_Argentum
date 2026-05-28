#ifndef YAML_MAP_LOADER_H
#define YAML_MAP_LOADER_H

#include <string>

#include "../../common/position.h"

#include "map.h"

// Map parseado del YAML + spawn point para los jugadores.
struct LoadedMap {
    Map map;
    Position playerSpawn;
};

// Carga el mapa desde un archivo YAML. Tira runtime_error si falla.
LoadedMap loadMapFromYaml(const std::string& path);

#endif  // YAML_MAP_LOADER_H
