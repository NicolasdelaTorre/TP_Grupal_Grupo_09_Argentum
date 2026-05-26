#include "template_registry.h"

#include <algorithm>
#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "editor_constants.h"

bool TemplateRegistry::load() {
    cities_.clear();
    biomes_.clear();
    obstacles_.clear();
    entries_.clear();
    walls_.clear();

    if (!std::filesystem::exists(TEMPLATES_CITIES_PATH) ||
        !std::filesystem::exists(TEMPLATES_BIOMES_PATH) ||
        !std::filesystem::exists(TEMPLATES_OBSTACLES_PATH) ||
        !std::filesystem::exists(TEMPLATES_ENTRIES_PATH) ||
        !std::filesystem::exists(TEMPLATES_WALLS_PATH)) {
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

    return !cities_.empty() && !biomes_.empty() && !obstacles_.empty() && !entries_.empty() &&
           !walls_.empty();
}

const std::vector<CityTemplate>& TemplateRegistry::cities() const { return cities_; }

const std::vector<BiomeTemplate>& TemplateRegistry::biomes() const { return biomes_; }

const std::vector<ObstacleTemplate>& TemplateRegistry::obstacles() const { return obstacles_; }

const std::vector<EntryTemplate>& TemplateRegistry::entries() const { return entries_; }

const std::vector<WallTemplate>& TemplateRegistry::walls() const { return walls_; }

const CityTemplate* TemplateRegistry::find_city(const std::string& id) const {
    const auto it = std::find_if(cities_.begin(), cities_.end(),
                                 [&id](const CityTemplate& city) { return city.id == id; });
    return it != cities_.end() ? &(*it) : nullptr;
}

const BiomeTemplate* TemplateRegistry::find_biome(const std::string& id) const {
    for (const auto& biome: biomes_) {
        if (biome.id == id) {
            return &biome;
        }
    }
    return nullptr;
}

const ObstacleTemplate* TemplateRegistry::find_obstacle(const std::string& id) const {
    for (const auto& obstacle: obstacles_) {
        if (obstacle.id == id) {
            return &obstacle;
        }
    }
    return nullptr;
}

const EntryTemplate* TemplateRegistry::find_entry(const std::string& id) const {
    for (const auto& entry: entries_) {
        if (entry.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

const WallTemplate* TemplateRegistry::find_wall(const std::string& id) const {
    for (const auto& wall: walls_) {
        if (wall.id == id) {
            return &wall;
        }
    }
    return nullptr;
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

        if (const auto colorNode = root["color"]) {
            obstacle.color = colorNode.as<std::string>();
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

        if (const auto colorNode = root["color"]) {
            entry.color = colorNode.as<std::string>();
        }

        const auto sizesNode = root["available_environment_sizes"];
        if (sizesNode && sizesNode.IsSequence()) {
            for (const auto& sizeOption: sizesNode) {
                EnvironmentSizeOption option;
                option.width = sizeOption["width"].as<int>();
                option.height = sizeOption["height"].as<int>();
                entry.environment_sizes.push_back(option);
            }
        }

        if (const auto floorNode = root["floor_color"]) {
            entry.floor_color = floorNode.as<std::string>();
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

        if (const auto colorNode = root["color"]) {
            wall.color = colorNode.as<std::string>();
        }

        walls_.push_back(wall);
        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}
