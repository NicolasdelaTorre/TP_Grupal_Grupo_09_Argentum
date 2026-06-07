#ifndef YAML_MAP_LOADER_H
#define YAML_MAP_LOADER_H

#include <string>
#include <vector>

#include "../../common/common_biome.h"
#include "../../common/position.h"

#include "map.h"

// Carga el mapa desde un archivo YAML. Tira runtime_error si falla.
Map loadMapFromYaml(const std::string& path);

#endif  // YAML_MAP_LOADER_H
