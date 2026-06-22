#ifndef YAML_MAP_LOADER_H
#define YAML_MAP_LOADER_H

#include <cstdint>
#include <string>
#include <vector>

#include "../../common/position.h"

#include "map.h"

namespace YAML {
class Node;
}

class YamlMapLoader {
public:
    // Carga el mapa desde un archivo YAML. Tira runtime_error si falla.
    static Map load(const std::string& path);

private:
    static uint8_t npcTypeFromString(const std::string& type);

    static void initializeDefaultCells(std::vector<Cell>& cells);

    static void applyObstacle(std::vector<Cell>& cells, uint16_t mapWidth, uint16_t mapHeight,
                              int16_t x, int16_t y, int16_t w, int16_t h, uint16_t obstacleId);

    static void applySafeZone(std::vector<Cell>& cells, uint16_t mapWidth, uint16_t mapHeight,
                              int16_t x, int16_t y, int16_t w, int16_t h);

    // transforma el grid de tiles de piso del editor en el textureId de cada celda.
    static void applyFloorGrid(std::vector<Cell>& cells, uint16_t mapWidth, uint16_t mapHeight,
                               const std::string& data);

    static std::vector<CreatureSpawn> parseSpawns(const YAML::Node& node);

    static Biome parseBiome(const YAML::Node& zone);

    // Arma la ruta de textura a partir del nombre  y la subcarpeta.
    static std::string textureRelPath(const YAML::Node& node, const char* subfolder);

    static void applyEnvironmentObstacles(std::vector<Cell>& cells, int16_t envWidth,
                                          int16_t envHeight, const YAML::Node& nodes,
                                          const char* subfolder = "",
                                          std::vector<PlacedObstacle>* out = nullptr);

    // Vuelca las salidas del environment en sus celda.
    static std::vector<Position> applyEnvironmentExits(std::vector<Cell>& cells, int16_t envWidth,
                                                       int16_t envHeight, const YAML::Node& exits,
                                                       std::vector<PlacedObstacle>* out = nullptr);

    // Resuelve el piso del environment con el flood-fill
    static void applyEnvironmentFloorAndExterior(std::vector<Cell>& cells, int16_t envWidth,
                                                 int16_t envHeight, const YAML::Node& walls,
                                                 const std::string& floorTexture);

    static std::vector<LoadedEnvironment> parseEnvironments(const YAML::Node& root);

    static const LoadedEnvironment* findEnvironmentById(
            const std::vector<LoadedEnvironment>& environments, const std::string& id);
};

#endif  // YAML_MAP_LOADER_H
