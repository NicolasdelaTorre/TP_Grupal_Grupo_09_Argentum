#include "template_registry.h"

#include <algorithm>
#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "editor_constants.h"

const EnvironmentTypeInfo& environment_type_info(const std::string& type) {
    static const EnvironmentTypeInfo Cueva{"cueva", "Cueva", "tiles/cueva.png"};
    static const EnvironmentTypeInfo Mazmorra{"mazmorra", "Mazmorra", "tiles/mazmorra.png"};
    if (type == Mazmorra.type) {
        return Mazmorra;
    }
    return Cueva;
}

bool TemplateRegistry::load() {
    cities_.clear();
    biomes_.clear();
    obstacles_.clear();
    entries_.clear();
    walls_.clear();
    exits_.clear();
    floors_.clear();

    if (!std::filesystem::exists(TEMPLATES_CITIES_PATH) ||
        !std::filesystem::exists(TEMPLATES_BIOMES_PATH) ||
        !std::filesystem::exists(TEMPLATES_OBSTACLES_PATH) ||
        !std::filesystem::exists(TEMPLATES_ENTRIES_PATH) ||
        !std::filesystem::exists(TEMPLATES_WALLS_PATH) ||
        !std::filesystem::exists(TEMPLATES_EXITS_PATH) ||
        !std::filesystem::exists(TEMPLATES_FLOORS_PATH)) {
        return false;
    }

    for (const auto& entry: std::filesystem::directory_iterator(TEMPLATES_CITIES_PATH)) {
        if (entry.path().extension() == ".yaml") {
            if (!load_city_file(entry.path().string())) {
                return false;
            }
        }
    }

    for (const auto& entry: std::filesystem::directory_iterator(TEMPLATES_BIOMES_PATH)) {
        if (entry.path().extension() == ".yaml") {
            if (!load_biome_file(entry.path().string())) {
                return false;
            }
        }
    }

    for (const auto& entry: std::filesystem::directory_iterator(TEMPLATES_OBSTACLES_PATH)) {
        if (entry.path().extension() == ".yaml") {
            if (!load_obstacle_file(entry.path().string())) {
                return false;
            }
        }
    }

    for (const auto& entry: std::filesystem::directory_iterator(TEMPLATES_ENTRIES_PATH)) {
        if (entry.path().extension() == ".yaml") {
            if (!load_entry_file(entry.path().string())) {
                return false;
            }
        }
    }

    for (const auto& entry: std::filesystem::directory_iterator(TEMPLATES_WALLS_PATH)) {
        if (entry.path().extension() == ".yaml") {
            if (!load_wall_file(entry.path().string())) {
                return false;
            }
        }
    }

    for (const auto& entry: std::filesystem::directory_iterator(TEMPLATES_EXITS_PATH)) {
        if (entry.path().extension() == ".yaml") {
            if (!load_exit_file(entry.path().string())) {
                return false;
            }
        }
    }

    for (const auto& entry: std::filesystem::directory_iterator(TEMPLATES_FLOORS_PATH)) {
        if (entry.path().extension() == ".yaml") {
            if (!load_floor_file(entry.path().string())) {
                return false;
            }
        }
    }

    return !cities_.empty() && !biomes_.empty() && !obstacles_.empty() && !entries_.empty() &&
           !walls_.empty() && !exits_.empty() && !floors_.empty();
}

const std::vector<CityTemplate>& TemplateRegistry::cities() const { return cities_; }

const std::vector<BiomeTemplate>& TemplateRegistry::biomes() const { return biomes_; }

const std::vector<ObstacleTemplate>& TemplateRegistry::obstacles() const { return obstacles_; }

const std::vector<EntryTemplate>& TemplateRegistry::entries() const { return entries_; }

const std::vector<WallTemplate>& TemplateRegistry::walls() const { return walls_; }

const std::vector<ExitTemplate>& TemplateRegistry::exits() const { return exits_; }

const std::vector<FloorTemplate>& TemplateRegistry::floors() const { return floors_; }

std::vector<std::string> TemplateRegistry::all_creatures() const {
    std::vector<std::string> creatures;
    for (const auto& biome: biomes_) {
        for (const auto& creature: biome.allowed_creatures) {
            if (std::find(creatures.begin(), creatures.end(), creature) == creatures.end()) {
                creatures.push_back(creature);
            }
        }
    }
    std::sort(creatures.begin(), creatures.end());
    return creatures;
}

const CityTemplate* TemplateRegistry::find_city(const std::string& id) const {
    const auto it = std::find_if(cities_.begin(), cities_.end(),
                                 [&id](const CityTemplate& city) { return city.id == id; });
    return it != cities_.end() ? &(*it) : nullptr;
}

const BiomeTemplate* TemplateRegistry::find_biome(const std::string& id) const {
    const auto it = std::find_if(biomes_.begin(), biomes_.end(),
                                 [&id](const BiomeTemplate& biome) { return biome.id == id; });
    return it != biomes_.end() ? &(*it) : nullptr;
}

const ObstacleTemplate* TemplateRegistry::find_obstacle(const std::string& id) const {
    const auto it =
            std::find_if(obstacles_.begin(), obstacles_.end(),
                         [&id](const ObstacleTemplate& obstacle) { return obstacle.id == id; });
    return it != obstacles_.end() ? &(*it) : nullptr;
}

const EntryTemplate* TemplateRegistry::find_entry(const std::string& id) const {
    const auto it = std::find_if(entries_.begin(), entries_.end(),
                                 [&id](const EntryTemplate& entry) { return entry.id == id; });
    return it != entries_.end() ? &(*it) : nullptr;
}

const WallTemplate* TemplateRegistry::find_wall(const std::string& id) const {
    const auto it = std::find_if(walls_.begin(), walls_.end(),
                                 [&id](const WallTemplate& wall) { return wall.id == id; });
    return it != walls_.end() ? &(*it) : nullptr;
}

const ExitTemplate* TemplateRegistry::find_exit(const std::string& id) const {
    const auto it = std::find_if(exits_.begin(), exits_.end(),
                                 [&id](const ExitTemplate& exit) { return exit.id == id; });
    return it != exits_.end() ? &(*it) : nullptr;
}

const FloorTemplate* TemplateRegistry::find_floor(const std::string& id) const {
    const auto it = std::find_if(floors_.begin(), floors_.end(),
                                 [&id](const FloorTemplate& floor) { return floor.id == id; });
    return it != floors_.end() ? &(*it) : nullptr;
}

const FloorTemplate* TemplateRegistry::find_floor_by_grid_value(int grid_value) const {
    const auto it = std::find_if(floors_.begin(), floors_.end(), [grid_value](const FloorTemplate& floor) {
        return floor.grid_value == grid_value;
    });
    return it != floors_.end() ? &(*it) : nullptr;
}

bool TemplateRegistry::load_city_file(const std::string& path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        CityTemplate city;
        city.id = root["template_id"].as<std::string>();
        city.name = root["name"].as<std::string>();

        const auto sizeNode = root["default_size"];
        if (sizeNode) {
            city.default_width = sizeNode["width"].as<int>();
            city.default_height = sizeNode["height"].as<int>();
        }

        if (const auto colorNode = root["color"]) {
            city.color = colorNode.as<std::string>();
        }

        const auto npcsNode = root["fixed_npcs"];
        if (npcsNode && npcsNode.IsSequence()) {
            for (const auto& npcNode: npcsNode) {
                NpcTemplate npc;
                npc.type = npcNode["type"].as<std::string>();
                npc.name = npcNode["name"].as<std::string>();
                const auto posNode = npcNode["position"];
                if (posNode && posNode.IsSequence() && posNode.size() >= 2) {
                    npc.relative_x = posNode[0].as<int>();
                    npc.relative_y = posNode[1].as<int>();
                }
                city.fixed_npcs.push_back(npc);
            }
        }

        const auto obstaclesNode = root["fixed_obstacles"];
        if (obstaclesNode && obstaclesNode.IsSequence()) {
            for (const auto& obstacleNode: obstaclesNode) {
                CityObstacleTemplate obstacle;
                obstacle.type = obstacleNode["type"].as<std::string>();
                const auto posNode = obstacleNode["position"];
                if (posNode && posNode.IsSequence() && posNode.size() >= 2) {
                    obstacle.relative_x = posNode[0].as<int>();
                    obstacle.relative_y = posNode[1].as<int>();
                }
                city.fixed_obstacles.push_back(obstacle);
            }
        }

        const auto floorsNode = root["fixed_floors"];
        if (floorsNode && floorsNode.IsSequence()) {
            for (const auto& floorNode: floorsNode) {
                CityFloorTemplate floor;
                floor.type = floorNode["type"].as<std::string>();
                const auto posNode = floorNode["position"];
                if (posNode && posNode.IsSequence() && posNode.size() >= 2) {
                    floor.relative_x = posNode[0].as<int>();
                    floor.relative_y = posNode[1].as<int>();
                }
                city.fixed_floors.push_back(floor);
            }
        }

        cities_.push_back(city);
        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}

bool TemplateRegistry::load_biome_file(const std::string& path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        BiomeTemplate biome;
        biome.id = root["template_id"].as<std::string>();
        biome.name = root["name"].as<std::string>();

        const auto sizeNode = root["default_size"];
        if (sizeNode) {
            biome.default_width = sizeNode["width"].as<int>();
            biome.default_height = sizeNode["height"].as<int>();
        }

        if (const auto textureNode = root["texture"]) {
            const auto filename = textureNode.as<std::string>();
            if (!filename.empty()) {
                const std::filesystem::path resolved =
                        std::filesystem::path(ASSETS_IMAGES_PATH) / filename;
                biome.texture = resolved.string();
            }
        }

        if (const auto colorNode = root["color"]) {
            biome.color = colorNode.as<std::string>();
        }

        const auto creaturesNode = root["allowed_creatures"];
        if (creaturesNode && creaturesNode.IsSequence()) {
            for (const auto& creatureNode: creaturesNode) {
                biome.allowed_creatures.push_back(creatureNode.as<std::string>());
            }
        }

        biomes_.push_back(biome);
        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}

bool TemplateRegistry::load_obstacle_file(const std::string& path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        ObstacleTemplate obstacle;
        obstacle.id = root["template_id"].as<std::string>();
        obstacle.name = root["name"].as<std::string>();

        const auto sizeNode = root["size"];
        if (sizeNode) {
            obstacle.width = sizeNode["width"].as<int>();
            obstacle.height = sizeNode["height"].as<int>();
        }

        if (const auto textureNode = root["texture"]) {
            const auto filename = textureNode.as<std::string>();
            if (!filename.empty()) {
                const std::filesystem::path resolved =
                        std::filesystem::path(ASSETS_IMAGES_PATH) / filename;
                obstacle.texture = resolved.string();
            }
        }

        if (const auto anchorNode = root["texture_anchor"]) {
            obstacle.texture_anchor = anchorNode.as<int>();
        }

        obstacles_.push_back(obstacle);
        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}

bool TemplateRegistry::load_entry_file(const std::string& path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        EntryTemplate entry;
        entry.id = root["template_id"].as<std::string>();
        entry.name = root["name"].as<std::string>();

        const auto sizeNode = root["size"];
        if (sizeNode) {
            entry.width = sizeNode["width"].as<int>();
            entry.height = sizeNode["height"].as<int>();
        }

        if (const auto textureNode = root["texture"]) {
            const auto filename = textureNode.as<std::string>();
            if (!filename.empty()) {
                const std::filesystem::path resolved =
                        std::filesystem::path(ASSETS_IMAGES_PATH) / filename;
                entry.texture = resolved.string();
            }
        }

        if (const auto envTypeNode = root["environment_type"]) {
            entry.environment_type = envTypeNode.as<std::string>();
        }

        entries_.push_back(entry);
        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}

bool TemplateRegistry::load_wall_file(const std::string& path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        WallTemplate wall;
        wall.id = root["template_id"].as<std::string>();
        wall.name = root["name"].as<std::string>();

        const auto sizeNode = root["size"];
        if (sizeNode) {
            wall.width = sizeNode["width"].as<int>();
            wall.height = sizeNode["height"].as<int>();
        }

        if (const auto textureNode = root["texture"]) {
            const auto filename = textureNode.as<std::string>();
            if (!filename.empty()) {
                const std::filesystem::path resolved =
                        std::filesystem::path(ASSETS_IMAGES_PATH) / filename;
                wall.texture = resolved.string();
            }
        }

        if (const auto envTypeNode = root["environment_type"]) {
            wall.environment_type = envTypeNode.as<std::string>();
        }

        walls_.push_back(wall);
        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}

bool TemplateRegistry::load_exit_file(const std::string& path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        ExitTemplate exit;
        exit.id = root["template_id"].as<std::string>();
        exit.name = root["name"].as<std::string>();

        const auto sizeNode = root["size"];
        if (sizeNode) {
            exit.width = sizeNode["width"].as<int>();
            exit.height = sizeNode["height"].as<int>();
        }

        if (const auto textureNode = root["texture"]) {
            const auto filename = textureNode.as<std::string>();
            if (!filename.empty()) {
                const std::filesystem::path resolved =
                        std::filesystem::path(ASSETS_IMAGES_PATH) / filename;
                exit.texture = resolved.string();
            }
        }

        exits_.push_back(exit);
        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}

bool TemplateRegistry::load_floor_file(const std::string& path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        FloorTemplate floor;
        floor.id = root["template_id"].as<std::string>();
        floor.name = root["name"].as<std::string>();

        if (const auto textureNode = root["texture"]) {
            const auto filename = textureNode.as<std::string>();
            if (!filename.empty()) {
                const std::filesystem::path resolved =
                        std::filesystem::path(ASSETS_IMAGES_PATH) / filename;
                floor.texture = resolved.string();
            }
        }

        if (const auto gridNode = root["grid_value"]) {
            floor.grid_value = gridNode.as<int>();
        }

        floors_.push_back(floor);
        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}
