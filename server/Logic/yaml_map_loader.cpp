#include "yaml_map_loader.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "../../common/DTOs.h"

namespace {

// Convierte el "type" de un NPC fijo del YAML al código de ObstacleType.
uint8_t npcTypeFromString(const std::string& type) {
    if (type == "priest")
        return static_cast<uint8_t>(ObstacleType::NPC_PRIEST);
    if (type == "merchant")
        return static_cast<uint8_t>(ObstacleType::NPC_MERCHANT);
    if (type == "banker")
        return static_cast<uint8_t>(ObstacleType::NPC_BANKER);
    return static_cast<uint8_t>(ObstacleType::NPC);
}

// Convierte el "type" del YAML al código de ObstacleType que va por la red.
uint8_t obstacleTypeFromString(const std::string& type) {
    if (type == "roca")
        return static_cast<uint8_t>(ObstacleType::ROCK);
    if (type == "piedra_pequenia")
        return static_cast<uint8_t>(ObstacleType::ROCK_SMALL);
    if (type == "piedra_grande")
        return static_cast<uint8_t>(ObstacleType::ROCK_LARGE);
    if (type == "arbol" || type == "arbol_grande" || type == "tronco")
        return static_cast<uint8_t>(ObstacleType::TREE);
    if (type == "arbusto")
        return static_cast<uint8_t>(ObstacleType::BUSH);
    if (type == "cactus")
        return static_cast<uint8_t>(ObstacleType::CACTUS);
    if (type == "lampara_ciudad")
        return static_cast<uint8_t>(ObstacleType::LAMP);
    if (type == "pila_maderas")
        return static_cast<uint8_t>(ObstacleType::WOOD);
    if (type == "carretilla_de_madera")
        return static_cast<uint8_t>(ObstacleType::CART);
    if (type == "molino")
        return static_cast<uint8_t>(ObstacleType::MILL);
    if (type == "pared_clara" || type == "pared_oscura" || type == "pared_piedra" ||
        type == "pilar")
        return static_cast<uint8_t>(ObstacleType::WALL);
    return static_cast<uint8_t>(ObstacleType::ROCK);
}

void initializeDefaultCells(std::vector<Cell>& cells) {
    for (auto& c: cells) {
        c.textureId = 0;
        c.obstacleId = 0;
        c.isWalkable = true;
        c.safeZone = false;
    }
}

void applyObstacle(std::vector<Cell>& cells, uint16_t mapWidth, uint16_t mapHeight, int16_t x,
                   int16_t y, int16_t w, int16_t h, uint16_t obstacleId) {
    for (int16_t tileY = y; tileY < y + h; tileY++) {
        for (int16_t tileX = x; tileX < x + w; tileX++) {
            if (tileX < 0 || tileY < 0 || tileX >= static_cast<int16_t>(mapWidth) ||
                tileY >= static_cast<int16_t>(mapHeight))
                continue;
            Cell& cell = cells[static_cast<size_t>(tileY) * mapWidth + tileX];
            cell.obstacleId = obstacleId;
            cell.isWalkable = false;
        }
    }
}

void applySafeZone(std::vector<Cell>& cells, uint16_t mapWidth, uint16_t mapHeight, int16_t x,
                   int16_t y, int16_t w, int16_t h) {
    for (int16_t tileY = y; tileY < y + h; tileY++) {
        for (int16_t tileX = x; tileX < x + w; tileX++) {
            if (tileX < 0 || tileY < 0 || tileX >= static_cast<int16_t>(mapWidth) ||
                tileY >= static_cast<int16_t>(mapHeight))
                continue;
            cells[static_cast<size_t>(tileY) * mapWidth + tileX].safeZone = true;
        }
    }
}

// Vuelca el grid de biomas por el editor en el textureId de cada celda.
void applyBiomeMap(std::vector<Cell>& cells, uint16_t mapWidth, uint16_t mapHeight,
                   const std::string& data) {
    std::istringstream stream(data);
    std::string line;
    uint16_t y = 0;
    while (y < mapHeight && std::getline(stream, line)) {
        for (uint16_t x = 0; x < mapWidth && x < line.size(); x++) {
            const char c = line[x];
            if (c >= '0' && c <= '9') {
                cells[static_cast<size_t>(y) * mapWidth + x].textureId =
                        static_cast<uint16_t>(c - '0');
            }
        }
        y++;
    }
}

std::vector<LoadedEnvironment> parseEnvironments(const YAML::Node& root) {
    std::vector<LoadedEnvironment> environments;
    if (!root["environments"]) {
        return environments;
    }

    for (const auto& envNode: root["environments"]) {
        LoadedEnvironment env;
        if (envNode["id"])
            env.id = envNode["id"].as<std::string>();
        if (envNode["name"])
            env.name = envNode["name"].as<std::string>();
        if (envNode["type"])
            env.type = envNode["type"].as<std::string>();
        if (envNode["size"] && envNode["size"].size() >= 2) {
            env.width = envNode["size"][0].as<int16_t>();
            env.height = envNode["size"][1].as<int16_t>();
        }

        const auto spawnNode = envNode["player_spawn"];
        if (spawnNode && spawnNode["position"] && spawnNode["position"].size() >= 2) {
            env.hasPlayerSpawn = true;
            env.playerSpawn.x = spawnNode["position"][0].as<int16_t>();
            env.playerSpawn.y = spawnNode["position"][1].as<int16_t>();
        }

        if (envNode["obstacles"]) {
            for (const auto& obsNode: envNode["obstacles"]) {
                EnvironmentObstacle obstacle;
                if (obsNode["type"])
                    obstacle.type = obsNode["type"].as<std::string>();
                if (obsNode["position"] && obsNode["position"].size() >= 2) {
                    obstacle.x = obsNode["position"][0].as<int16_t>();
                    obstacle.y = obsNode["position"][1].as<int16_t>();
                }
                if (obsNode["size"] && obsNode["size"].size() >= 2) {
                    obstacle.width = obsNode["size"][0].as<int16_t>();
                    obstacle.height = obsNode["size"][1].as<int16_t>();
                }
                env.obstacles.push_back(obstacle);
            }
        }

        if (envNode["walls"]) {
            for (const auto& wallNode: envNode["walls"]) {
                EnvironmentObstacle wall;
                if (wallNode["template"])
                    wall.type = wallNode["template"].as<std::string>();
                if (wallNode["position"] && wallNode["position"].size() >= 2) {
                    wall.x = wallNode["position"][0].as<int16_t>();
                    wall.y = wallNode["position"][1].as<int16_t>();
                }
                if (wallNode["size"] && wallNode["size"].size() >= 2) {
                    wall.width = wallNode["size"][0].as<int16_t>();
                    wall.height = wallNode["size"][1].as<int16_t>();
                }
                env.walls.push_back(wall);
            }
        }

        if (envNode["floor_color"])
            env.floorColor = envNode["floor_color"].as<std::string>();

        environments.push_back(std::move(env));
    }

    return environments;
}

}  // namespace

LoadedMap loadMapFromYaml(const std::string& path) {
    YAML::Node root;
    try {
        root = YAML::LoadFile(path);
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("YAML Loader Error: " + std::string(e.what()));
    }

    if (!root["map"] || !root["map"]["width"] || !root["map"]["height"]) {
        throw std::runtime_error("YAML Loader Error: missing map.width or map.height");
    }

    uint16_t width = root["map"]["width"].as<uint16_t>();
    uint16_t height = root["map"]["height"].as<uint16_t>();

    std::vector<Cell> cells(static_cast<size_t>(width) * height);
    initializeDefaultCells(cells);

    if (root["biome_map"] && root["biome_map"]["data"]) {
        applyBiomeMap(cells, width, height, root["biome_map"]["data"].as<std::string>());
    }

    // Obstáculos: cada celda guarda el código de ObstacleType
    if (root["obstacles"]) {
        for (const auto& obs: root["obstacles"]) {
            std::string type = obs["type"].as<std::string>();
            uint8_t typeCode = obstacleTypeFromString(type);
            int16_t ox = obs["position"][0].as<int16_t>();
            int16_t oy = obs["position"][1].as<int16_t>();
            int16_t ow = obs["size"][0].as<int16_t>();
            int16_t oh = obs["size"][1].as<int16_t>();
            applyObstacle(cells, width, height, ox, oy, ow, oh, typeCode);
        }
    }

    // Zonas tipo ciudad -> safe zone + fixed_npcs bloquean su celda.
    // Biomes y otros tipos por ahora se ignoran.
    if (root["zones"]) {
        for (const auto& zone: root["zones"]) {
            std::string type = zone["type"].as<std::string>();
            if (type != "city")
                continue;
            int16_t zx = zone["area"]["x"].as<int16_t>();
            int16_t zy = zone["area"]["y"].as<int16_t>();
            int16_t zw = zone["area"]["width"].as<int16_t>();
            int16_t zh = zone["area"]["height"].as<int16_t>();
            applySafeZone(cells, width, height, zx, zy, zw, zh);

            if (zone["fixed_npcs"]) {
                for (const auto& npc: zone["fixed_npcs"]) {
                    int16_t nx = npc["position"][0].as<int16_t>();
                    int16_t ny = npc["position"][1].as<int16_t>();
                    const std::string npcType =
                            npc["type"] ? npc["type"].as<std::string>() : std::string();
                    applyObstacle(cells, width, height, nx, ny, 1, 1, npcTypeFromString(npcType));
                }
            }
        }
    }

    // Entries (portales a cuevas): bloquean su rectángulo en el mapa principal
    // y, además, se guardan con su relación al environment.
    std::vector<LoadedEntry> entries;
    if (root["entries"]) {
        for (const auto& entry: root["entries"]) {
            int16_t ex = entry["position"][0].as<int16_t>();
            int16_t ey = entry["position"][1].as<int16_t>();
            int16_t ew = entry["size"][0].as<int16_t>();
            int16_t eh = entry["size"][1].as<int16_t>();
            applyObstacle(cells, width, height, ex, ey, ew, eh,
                          static_cast<uint8_t>(ObstacleType::ENTRY));

            LoadedEntry loadedEntry;
            if (entry["id"])
                loadedEntry.id = entry["id"].as<std::string>();
            if (entry["type"])
                loadedEntry.type = entry["type"].as<std::string>();
            if (entry["environment"])
                loadedEntry.environmentId = entry["environment"].as<std::string>();
            loadedEntry.x = ex;
            loadedEntry.y = ey;
            loadedEntry.width = ew;
            loadedEntry.height = eh;
            entries.push_back(std::move(loadedEntry));
        }
    }

    Position spawn{static_cast<int16_t>(width / 2), static_cast<int16_t>(height / 2)};
    if (root["player_spawn"] && root["player_spawn"]["position"]) {
        spawn.x = root["player_spawn"]["position"][0].as<int16_t>();
        spawn.y = root["player_spawn"]["position"][1].as<int16_t>();
    }

    std::vector<LoadedEnvironment> environments = parseEnvironments(root);

    std::cout << "Map loaded (" << width << "x" << height << "), spawn at (" << spawn.x << ", "
              << spawn.y << "), " << entries.size() << " entr(y/ies), " << environments.size()
              << " environment(s)" << std::endl;

    return LoadedMap{Map(width, height, std::move(cells)), spawn, std::move(entries),
                     std::move(environments)};
}
