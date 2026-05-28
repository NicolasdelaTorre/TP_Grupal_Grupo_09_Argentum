#include "yaml_map_loader.h"

#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace {

void initializeDefaultCells(std::vector<Cell>& cells) {
    for (auto& c: cells) {
        c.textureId = 0;
        c.obstacleId = 0;
        c.isWalkable = true;
        c.safeZone = false;
    }
}

void applyObstacle(std::vector<Cell>& cells, uint16_t mapWidth, uint16_t mapHeight,
                   int16_t x, int16_t y, int16_t w, int16_t h, uint16_t obstacleId) {
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

void applySafeZone(std::vector<Cell>& cells, uint16_t mapWidth, uint16_t mapHeight,
                   int16_t x, int16_t y, int16_t w, int16_t h) {
    for (int16_t tileY = y; tileY < y + h; tileY++) {
        for (int16_t tileX = x; tileX < x + w; tileX++) {
            if (tileX < 0 || tileY < 0 || tileX >= static_cast<int16_t>(mapWidth) ||
                tileY >= static_cast<int16_t>(mapHeight))
                continue;
            cells[static_cast<size_t>(tileY) * mapWidth + tileX].safeZone = true;
        }
    }
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

    // Obstáculos: cada uno reserva un rectángulo bloqueado
    uint16_t nextObstacleId = 1;
    if (root["obstacles"]) {
        for (const auto& obs: root["obstacles"]) {
            int16_t ox = obs["position"][0].as<int16_t>();
            int16_t oy = obs["position"][1].as<int16_t>();
            int16_t ow = obs["size"][0].as<int16_t>();
            int16_t oh = obs["size"][1].as<int16_t>();
            applyObstacle(cells, width, height, ox, oy, ow, oh, nextObstacleId++);
        }
    }

    // Zonas tipo ciudad -> safe zone. Forest y otros tipos por ahora se ignoran.
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
        }
    }

    Position spawn{static_cast<int16_t>(width / 2), static_cast<int16_t>(height / 2)};
    if (root["player_spawn"] && root["player_spawn"]["position"]) {
        spawn.x = root["player_spawn"]["position"][0].as<int16_t>();
        spawn.y = root["player_spawn"]["position"][1].as<int16_t>();
    }

    std::cout << "Map loaded (" << width << "x" << height << "), spawn at (" << spawn.x
              << ", " << spawn.y << ")" << std::endl;

    return LoadedMap{Map(width, height, std::move(cells)), spawn};
}
