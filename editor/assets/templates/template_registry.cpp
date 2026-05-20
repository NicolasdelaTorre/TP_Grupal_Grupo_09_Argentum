#include "template_registry.h"

#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "editor_constants.h"

bool TemplateRegistry::load() {
    cities_.clear();
    forests_.clear();

    if (!std::filesystem::exists(TEMPLATES_CITIES_PATH) ||
        !std::filesystem::exists(TEMPLATES_FORESTS_PATH)) {
        return false;
    }

    for (const auto& entry: std::filesystem::directory_iterator(TEMPLATES_CITIES_PATH)) {
        if (entry.path().extension() == ".yaml") {
            if (!load_city_file(entry.path().string())) {
                return false;
            }
        }
    }

    for (const auto& entry: std::filesystem::directory_iterator(TEMPLATES_FORESTS_PATH)) {
        if (entry.path().extension() == ".yaml") {
            if (!load_forest_file(entry.path().string())) {
                return false;
            }
        }
    }

    return !cities_.empty() && !forests_.empty();
}

const std::vector<CityTemplate>& TemplateRegistry::cities() const { return cities_; }

const std::vector<ForestTemplate>& TemplateRegistry::forests() const { return forests_; }

const CityTemplate* TemplateRegistry::find_city(const std::string& id) const {
    for (const auto& city: cities_) {
        if (city.id == id) {
            return &city;
        }
    }
    return nullptr;
}

const ForestTemplate* TemplateRegistry::find_forest(const std::string& id) const {
    for (const auto& forest: forests_) {
        if (forest.id == id) {
            return &forest;
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

bool TemplateRegistry::load_forest_file(const std::string& path) {
    try {
        YAML::Node root = YAML::LoadFile(path);
        ForestTemplate forest;
        forest.id = root["template_id"].as<std::string>();
        forest.name = root["name"].as<std::string>();

        const auto sizeNode = root["default_size"];
        if (sizeNode) {
            forest.default_width = sizeNode["width"].as<int>();
            forest.default_height = sizeNode["height"].as<int>();
        }

        const auto creaturesNode = root["allowed_creatures"];
        if (creaturesNode && creaturesNode.IsSequence()) {
            for (const auto& creatureNode: creaturesNode) {
                forest.allowed_creatures.push_back(creatureNode.as<std::string>());
            }
        }

        forests_.push_back(forest);
        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}
