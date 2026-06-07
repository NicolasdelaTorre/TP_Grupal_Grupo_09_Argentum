#include "yaml_map_loader.h"

#include <iostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "../../common/DTOs.h"
#include "../../common/common_tiles.h"

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
    if (type == "banco")
        return static_cast<uint8_t>(ObstacleType::BANK);
    if (type == "casa_madera_azul")
        return static_cast<uint8_t>(ObstacleType::HOUSE_BLUE);
    if (type == "casa_madera_roja")
        return static_cast<uint8_t>(ObstacleType::HOUSE_RED);
    if (type == "casa_nevada")
        return static_cast<uint8_t>(ObstacleType::HOUSE_SNOW);
    if (type == "cerca_madera")
        return static_cast<uint8_t>(ObstacleType::FENCE);
    if (type == "diana")
        return static_cast<uint8_t>(ObstacleType::TARGET);
    if (type == "fardo_heno")
        return static_cast<uint8_t>(ObstacleType::HAYBALE);
    if (type == "fuente")
        return static_cast<uint8_t>(ObstacleType::FOUNTAIN);
    if (type == "herrero")
        return static_cast<uint8_t>(ObstacleType::BLACKSMITH);
    if (type == "hotel")
        return static_cast<uint8_t>(ObstacleType::HOTEL);
    if (type == "iglesia")
        return static_cast<uint8_t>(ObstacleType::CHURCH);
    if (type == "munieco_entrenamiento")
        return static_cast<uint8_t>(ObstacleType::TRAINING_DUMMY);
    if (type == "pared_clara" || type == "pared_oscura" || type == "pared_piedra" ||
        type == "pilar")
        return static_cast<uint8_t>(ObstacleType::WALL);
    return static_cast<uint8_t>(ObstacleType::ROCK);
}

void initializeDefaultCells(std::vector<Cell>& cells) {
    for (auto& c: cells) {
        c.textureId = 0;
        c.obstacleId = 0;
        c.playerId = 0;
        c.npcId = 0;
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

// Vuelca el grid de tiles de piso del editor en el textureId de cada celda.
// La clave YAML sigue siendo "biome_map" por compatibilidad con mapas existentes.
void applyFloorGrid(std::vector<Cell>& cells, uint16_t mapWidth, uint16_t mapHeight,
                    const std::string& data) {
    std::istringstream stream(data);
    std::string line;
    uint16_t y = 0;
    while (y < mapHeight && std::getline(stream, line)) {
        for (uint16_t x = 0; x < mapWidth && x < line.size(); x++) {
            const char c = line[x];
            // Codificación base 36 (0-9 y a-z). Cada carácter es el grid_value
            // del tile de piso, que se envía al cliente como textureId.
            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                cells[static_cast<size_t>(y) * mapWidth + x].textureId =
                        static_cast<uint16_t>(grid_char_to_value(c));
            }
        }
        y++;
    }
}

// Lee la lista de spawns de criaturas de un nodo ("spawns": creature +
// max_population). La usan tanto los biomas como los environments.
std::vector<CreatureSpawn> parseSpawns(const YAML::Node& node) {
    std::vector<CreatureSpawn> spawns;
    if (!node["spawns"])
        return spawns;
    for (const auto& spawnNode: node["spawns"]) {
        CreatureSpawn spawn;
        if (spawnNode["creature"])
            spawn.creature = spawnNode["creature"].as<std::string>();
        if (spawnNode["max_population"])
            spawn.maxPopulation = spawnNode["max_population"].as<uint16_t>();
        spawns.push_back(std::move(spawn));
    }
    return spawns;
}

// Parsea una zona de tipo "biome": tipo (a partir del template), área
// (posición + tamaño) y la lista de spawns de criaturas.
Biome parseBiome(const YAML::Node& zone) {
    Biome biome;
    if (zone["template"])
        biome.type = zone["template"].as<std::string>();

    const auto area = zone["area"];
    if (!area || !area["x"] || !area["y"] || !area["width"] || !area["height"]) {
        throw std::runtime_error("YAML Loader Error: biome zone missing area");
    }
    biome.position.x = area["x"].as<int16_t>();
    biome.position.y = area["y"].as<int16_t>();
    biome.width = area["width"].as<int16_t>();
    biome.height = area["height"].as<int16_t>();

    biome.spawns = parseSpawns(zone);

    return biome;
}

// Aplica una lista de obstáculos/paredes del environment sobre sus celdas.
// `typeKey` es la clave del YAML que tiene el tipo ("type" u "template").
void applyEnvironmentObstacles(std::vector<Cell>& cells, int16_t envWidth, int16_t envHeight,
                               const YAML::Node& nodes, const char* typeKey) {
    if (!nodes)
        return;
    for (const auto& node: nodes) {
        const std::string type = node[typeKey] ? node[typeKey].as<std::string>() : std::string();
        int16_t x = 0;
        int16_t y = 0;
        int16_t w = 1;
        int16_t h = 1;
        if (node["position"] && node["position"].size() >= 2) {
            x = node["position"][0].as<int16_t>();
            y = node["position"][1].as<int16_t>();
        }
        if (node["size"] && node["size"].size() >= 2) {
            w = node["size"][0].as<int16_t>();
            h = node["size"][1].as<int16_t>();
        }
        applyObstacle(cells, static_cast<uint16_t>(envWidth), static_cast<uint16_t>(envHeight), x,
                      y, w, h, obstacleTypeFromString(type));
    }
}

void markEnvironmentExteriorAsBlocked(std::vector<Cell>& cells, int16_t envWidth,
                                      int16_t envHeight, const YAML::Node& walls) {
    if (envWidth <= 0 || envHeight <= 0 || cells.empty())
        return;

    const auto index = [envWidth](int16_t x, int16_t y) {
        return static_cast<size_t>(y) * static_cast<size_t>(envWidth) + static_cast<size_t>(x);
    };

    std::vector<bool> isWall(cells.size(), false);
    if (walls) {
        for (const auto& wall: walls) {
            int16_t x = 0;
            int16_t y = 0;
            int16_t w = 1;
            int16_t h = 1;
            if (wall["position"] && wall["position"].size() >= 2) {
                x = wall["position"][0].as<int16_t>();
                y = wall["position"][1].as<int16_t>();
            }
            if (wall["size"] && wall["size"].size() >= 2) {
                w = wall["size"][0].as<int16_t>();
                h = wall["size"][1].as<int16_t>();
            }
            for (int16_t dy = 0; dy < h; ++dy) {
                for (int16_t dx = 0; dx < w; ++dx) {
                    const int16_t cx = static_cast<int16_t>(x + dx);
                    const int16_t cy = static_cast<int16_t>(y + dy);
                    if (cx < 0 || cy < 0 || cx >= envWidth || cy >= envHeight)
                        continue;
                    isWall[index(cx, cy)] = true;
                }
            }
        }
    }

    std::vector<bool> exterior(cells.size(), false);
    std::queue<Position> pending;

    const auto enqueue = [&](int16_t x, int16_t y) {
        if (x < 0 || y < 0 || x >= envWidth || y >= envHeight)
            return;
        const size_t idx = index(x, y);
        if (exterior[idx] || isWall[idx])
            return;
        exterior[idx] = true;
        pending.push({x, y});
    };

    for (int16_t x = 0; x < envWidth; ++x) {
        enqueue(x, 0);
        enqueue(x, static_cast<int16_t>(envHeight - 1));
    }
    for (int16_t y = 0; y < envHeight; ++y) {
        enqueue(0, y);
        enqueue(static_cast<int16_t>(envWidth - 1), y);
    }

    while (!pending.empty()) {
        const Position current = pending.front();
        pending.pop();

        enqueue(static_cast<int16_t>(current.x + 1), current.y);
        enqueue(static_cast<int16_t>(current.x - 1), current.y);
        enqueue(current.x, static_cast<int16_t>(current.y + 1));
        enqueue(current.x, static_cast<int16_t>(current.y - 1));
    }

    bool hasInterior = false;
    for (size_t i = 0; i < cells.size(); ++i) {
        if (!isWall[i] && !exterior[i]) {
            hasInterior = true;
            break;
        }
    }
    if (!hasInterior)
        return;

    // Mismo criterio visual que el editor: si hay un recinto cerrado por paredes,
    // todo lo alcanzable desde el borde sin cruzar paredes es exterior.
    for (size_t i = 0; i < cells.size(); ++i) {
        if (exterior[i]) {
            cells[i].isWalkable = false;
            cells[i].textureId = EXTERIOR_TILE_VALUE;
        }
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
        if (env.width > 0 && env.height > 0) {
            env.cells.resize(static_cast<size_t>(env.width) * static_cast<size_t>(env.height));
            initializeDefaultCells(env.cells);
        }

        const auto spawnNode = envNode["player_spawn"];
        if (!spawnNode || !spawnNode["position"] || spawnNode["position"].size() < 2) {
            throw std::runtime_error(
                    "YAML Loader Error: environment missing player_spawn.position");
        }
        env.playerSpawn.x = spawnNode["position"][0].as<int16_t>();
        env.playerSpawn.y = spawnNode["position"][1].as<int16_t>();

        // Obstáculos y paredes se vuelcan directamente en las celdas del
        // environment (las paredes traen su tipo en "template" en vez de "type").
        if (!env.cells.empty()) {
            applyEnvironmentObstacles(env.cells, env.width, env.height, envNode["obstacles"],
                                      "type");
            applyEnvironmentObstacles(env.cells, env.width, env.height, envNode["walls"],
                                      "template");
            markEnvironmentExteriorAsBlocked(env.cells, env.width, env.height, envNode["walls"]);
        }

        env.spawns = parseSpawns(envNode);

        if (envNode["floor_color"])
            env.floorColor = envNode["floor_color"].as<std::string>();

        environments.push_back(std::move(env));
    }

    return environments;
}

const LoadedEnvironment* findEnvironmentById(const std::vector<LoadedEnvironment>& environments,
                                             const std::string& id) {
    for (const auto& environment: environments) {
        if (environment.id == id)
            return &environment;
    }
    return nullptr;
}

}  // namespace

Map loadMapFromYaml(const std::string& path) {
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
        applyFloorGrid(cells, width, height, root["biome_map"]["data"].as<std::string>());
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

    // Zonas: las de tipo ciudad -> safe zone + fixed_npcs bloquean su celda;
    // las de tipo bioma se cargan con su posición, tamaño y spawns de criaturas.
    std::vector<Biome> biomes;
    if (root["zones"]) {
        for (const auto& zone: root["zones"]) {
            std::string type = zone["type"].as<std::string>();
            if (type == "biome") {
                biomes.push_back(parseBiome(zone));
                continue;
            }
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
                    // markNpcOccupancy(cells, width, height, nx, ny, npcType);
                }
            }
        }
    }

    std::vector<LoadedEnvironment> environments = parseEnvironments(root);

    // Entries (portales a cuevas): bloquean su rectángulo en el mapa principal
    // y, además, guardan una copia del environment al que llevan.
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
            if (entry["environment"]) {
                const std::string environmentId = entry["environment"].as<std::string>();
                const LoadedEnvironment* environment =
                        findEnvironmentById(environments, environmentId);
                if (!environment) {
                    throw std::runtime_error(
                            "YAML Loader Error: entry references unknown environment '" +
                            environmentId + "'");
                }
                loadedEntry.environment = *environment;
            }
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

    std::cout << "Map loaded (" << width << "x" << height << "), spawn at (" << spawn.x << ", "
              << spawn.y << "), " << entries.size() << " entr(y/ies), " << environments.size()
              << " environment(s), " << biomes.size() << " biome(s)" << std::endl;

    return Map(width, height, std::move(cells), spawn, std::move(entries), std::move(biomes));
}
