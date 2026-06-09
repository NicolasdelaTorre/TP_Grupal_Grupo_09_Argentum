#include "yaml_map_io.h"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>

#include "../../common/common_tiles.h"

void YamlMapIO::write_floor_grid(YAML::Emitter& out, const MapDocument& document) {
    const int width = document.map.width;
    const int height = document.map.height;

    // Cada carácter representa el grid_value del tile de piso.
    std::string grid;
    grid.reserve(static_cast<size_t>(width + 1) * height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const size_t idx = static_cast<size_t>(y) * width + x;
            const uint8_t value = idx < document.biome_grid.size() ? document.biome_grid[idx] : 0;
            grid.push_back(grid_value_to_char(value));
        }
        if (y + 1 < height) {
            grid.push_back('\n');
        }
    }

    out << YAML::Key << "biome_map" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "width" << YAML::Value << width;
    out << YAML::Key << "height" << YAML::Value << height;
    out << YAML::Key << "data" << YAML::Value << YAML::Literal << grid;
    out << YAML::EndMap;
}

bool YamlMapIO::save(const MapDocument& document, const std::string& path) {
    try {
        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "version" << YAML::Value << document.version;

        out << YAML::Key << "map" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "id" << YAML::Value << document.map.id;
        out << YAML::Key << "name" << YAML::Value << document.map.name;
        out << YAML::Key << "width" << YAML::Value << document.map.width;
        out << YAML::Key << "height" << YAML::Value << document.map.height;
        out << YAML::EndMap;

        // Grid de tiles de piso pre-calculado.
        if (!document.biome_grid.empty() && document.map.width > 0 && document.map.height > 0) {
            write_floor_grid(out, document);
        }

        if (document.player_spawn.placed) {
            out << YAML::Key << "player_spawn" << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "position" << YAML::Value;
            out << YAML::Flow << YAML::BeginSeq << document.player_spawn.x
                << document.player_spawn.y << YAML::EndSeq;
            out << YAML::EndMap;
        }

        if (!document.obstacles.empty()) {
            out << YAML::Key << "obstacles" << YAML::Value << YAML::BeginSeq;
            for (const auto& obstacle: document.obstacles) {
                out << YAML::BeginMap;
                out << YAML::Key << "id" << YAML::Value << obstacle.id;
                out << YAML::Key << "type" << YAML::Value << obstacle.type;
                out << YAML::Key << "position" << YAML::Value;
                out << YAML::Flow << YAML::BeginSeq << obstacle.x << obstacle.y << YAML::EndSeq;
                out << YAML::Key << "size" << YAML::Value;
                out << YAML::Flow << YAML::BeginSeq << obstacle.width << obstacle.height
                    << YAML::EndSeq;
                if (!obstacle.texture.empty()) {
                    out << YAML::Key << "texture" << YAML::Value << obstacle.texture;
                }
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
        }

        if (!document.zones.empty()) {
            out << YAML::Key << "zones" << YAML::Value << YAML::BeginSeq;
            for (const auto& zone: document.zones) {
                out << YAML::BeginMap;
                out << YAML::Key << "id" << YAML::Value << zone.id;
                out << YAML::Key << "type" << YAML::Value << zone.type;
                out << YAML::Key << "template" << YAML::Value << zone.template_id;
                out << YAML::Key << "area" << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "x" << YAML::Value << zone.area_x;
                out << YAML::Key << "y" << YAML::Value << zone.area_y;
                out << YAML::Key << "width" << YAML::Value << zone.area_width;
                out << YAML::Key << "height" << YAML::Value << zone.area_height;
                out << YAML::EndMap;

                if (zone.type == "biome" && !zone.texture.empty()) {
                    out << YAML::Key << "texture" << YAML::Value << zone.texture;
                }

                if (zone.type == "biome" && !zone.spawns.empty()) {
                    out << YAML::Key << "spawns" << YAML::Value << YAML::BeginSeq;
                    for (const auto& spawn: zone.spawns) {
                        out << YAML::BeginMap;
                        out << YAML::Key << "creature" << YAML::Value << spawn.creature;
                        out << YAML::Key << "max_population" << YAML::Value << spawn.max_population;
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;
                }

                if (zone.type == "city" && !zone.fixed_npcs.empty()) {
                    out << YAML::Key << "fixed_npcs" << YAML::Value << YAML::BeginSeq;
                    for (const auto& npc: zone.fixed_npcs) {
                        out << YAML::BeginMap;
                        out << YAML::Key << "type" << YAML::Value << npc.type;
                        out << YAML::Key << "name" << YAML::Value << npc.name;
                        out << YAML::Key << "position" << YAML::Value;
                        out << YAML::Flow << YAML::BeginSeq << npc.x << npc.y << YAML::EndSeq;
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;
                }

                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
        }

        if (!document.entries.empty()) {
            out << YAML::Key << "entries" << YAML::Value << YAML::BeginSeq;
            for (const auto& entry: document.entries) {
                out << YAML::BeginMap;
                out << YAML::Key << "id" << YAML::Value << entry.id;
                out << YAML::Key << "type" << YAML::Value << entry.type;
                out << YAML::Key << "environment" << YAML::Value << entry.environment_id;
                out << YAML::Key << "position" << YAML::Value;
                out << YAML::Flow << YAML::BeginSeq << entry.x << entry.y << YAML::EndSeq;
                out << YAML::Key << "size" << YAML::Value;
                out << YAML::Flow << YAML::BeginSeq << entry.width << entry.height << YAML::EndSeq;
                if (!entry.texture.empty()) {
                    out << YAML::Key << "texture" << YAML::Value << entry.texture;
                }
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
        }

        if (!document.environments.empty()) {
            out << YAML::Key << "environments" << YAML::Value << YAML::BeginSeq;
            for (const auto& env: document.environments) {
                out << YAML::BeginMap;
                out << YAML::Key << "id" << YAML::Value << env.id;
                out << YAML::Key << "name" << YAML::Value << env.name;
                out << YAML::Key << "type" << YAML::Value << env.type;
                out << YAML::Key << "size" << YAML::Value;
                out << YAML::Flow << YAML::BeginSeq << env.width << env.height << YAML::EndSeq;

                if (env.player_spawn.placed) {
                    out << YAML::Key << "player_spawn" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "position" << YAML::Value;
                    out << YAML::Flow << YAML::BeginSeq << env.player_spawn.x << env.player_spawn.y
                        << YAML::EndSeq;
                    out << YAML::EndMap;
                }

                if (!env.obstacles.empty()) {
                    out << YAML::Key << "obstacles" << YAML::Value << YAML::BeginSeq;
                    for (const auto& obstacle: env.obstacles) {
                        out << YAML::BeginMap;
                        out << YAML::Key << "id" << YAML::Value << obstacle.id;
                        out << YAML::Key << "type" << YAML::Value << obstacle.type;
                        out << YAML::Key << "position" << YAML::Value;
                        out << YAML::Flow << YAML::BeginSeq << obstacle.x << obstacle.y
                            << YAML::EndSeq;
                        out << YAML::Key << "size" << YAML::Value;
                        out << YAML::Flow << YAML::BeginSeq << obstacle.width << obstacle.height
                            << YAML::EndSeq;
                        if (!obstacle.texture.empty()) {
                            out << YAML::Key << "texture" << YAML::Value << obstacle.texture;
                        }
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;
                }

                if (!env.walls.empty()) {
                    out << YAML::Key << "walls" << YAML::Value << YAML::BeginSeq;
                    for (const auto& wall: env.walls) {
                        out << YAML::BeginMap;
                        out << YAML::Key << "id" << YAML::Value << wall.id;
                        out << YAML::Key << "template" << YAML::Value << wall.template_id;
                        out << YAML::Key << "position" << YAML::Value;
                        out << YAML::Flow << YAML::BeginSeq << wall.x << wall.y << YAML::EndSeq;
                        out << YAML::Key << "size" << YAML::Value;
                        out << YAML::Flow << YAML::BeginSeq << wall.width << wall.height
                            << YAML::EndSeq;
                        if (!wall.texture.empty()) {
                            out << YAML::Key << "texture" << YAML::Value << wall.texture;
                        }
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;
                }

                if (!env.exits.empty()) {
                    out << YAML::Key << "exits" << YAML::Value << YAML::BeginSeq;
                    for (const auto& exit: env.exits) {
                        out << YAML::BeginMap;
                        out << YAML::Key << "id" << YAML::Value << exit.id;
                        out << YAML::Key << "template" << YAML::Value << exit.template_id;
                        out << YAML::Key << "position" << YAML::Value;
                        out << YAML::Flow << YAML::BeginSeq << exit.x << exit.y << YAML::EndSeq;
                        out << YAML::Key << "size" << YAML::Value;
                        out << YAML::Flow << YAML::BeginSeq << exit.width << exit.height
                            << YAML::EndSeq;
                        if (!exit.texture.empty()) {
                            out << YAML::Key << "texture" << YAML::Value << exit.texture;
                        }
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;
                }

                if (!env.spawns.empty()) {
                    out << YAML::Key << "spawns" << YAML::Value << YAML::BeginSeq;
                    for (const auto& spawn: env.spawns) {
                        out << YAML::BeginMap;
                        out << YAML::Key << "creature" << YAML::Value << spawn.creature;
                        out << YAML::Key << "max_population" << YAML::Value << spawn.max_population;
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;
                }

                if (!env.floor_texture.empty()) {
                    out << YAML::Key << "floor_texture" << YAML::Value << env.floor_texture;
                }

                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
        }

        out << YAML::EndMap;

        std::ofstream file(path);
        if (!file.is_open()) {
            return false;
        }
        file << out.c_str();
        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}

PlayerSpawn YamlMapIO::read_player_spawn(const YAML::Node& node) {
    PlayerSpawn spawn;
    if (node && node["position"] && node["position"].size() >= 2) {
        spawn.x = node["position"][0].as<int>();
        spawn.y = node["position"][1].as<int>();
        spawn.placed = true;
    }
    return spawn;
}

Obstacle YamlMapIO::read_obstacle(const YAML::Node& node) {
    Obstacle obstacle;
    obstacle.id = node["id"] ? node["id"].as<std::string>() : std::string();
    obstacle.type = node["type"] ? node["type"].as<std::string>() : std::string();
    if (node["position"] && node["position"].size() >= 2) {
        obstacle.x = node["position"][0].as<int>();
        obstacle.y = node["position"][1].as<int>();
    }
    if (node["size"] && node["size"].size() >= 2) {
        obstacle.width = node["size"][0].as<int>();
        obstacle.height = node["size"][1].as<int>();
    }
    if (node["texture"]) {
        obstacle.texture = node["texture"].as<std::string>();
    }
    return obstacle;
}

Zone YamlMapIO::read_zone(const YAML::Node& node) {
    Zone zone;
    zone.id = node["id"] ? node["id"].as<std::string>() : std::string();
    zone.type = node["type"] ? node["type"].as<std::string>() : std::string();
    zone.template_id = node["template"] ? node["template"].as<std::string>() : std::string();
    if (node["area"]) {
        const auto& area = node["area"];
        zone.area_x = area["x"] ? area["x"].as<int>() : 0;
        zone.area_y = area["y"] ? area["y"].as<int>() : 0;
        zone.area_width = area["width"] ? area["width"].as<int>() : 0;
        zone.area_height = area["height"] ? area["height"].as<int>() : 0;
    }
    if (node["texture"]) {
        zone.texture = node["texture"].as<std::string>();
    }
    if (node["spawns"]) {
        for (const auto& spawn_node: node["spawns"]) {
            CreatureSpawn spawn;
            spawn.creature = spawn_node["creature"] ? spawn_node["creature"].as<std::string>() :
                                                      std::string();
            spawn.max_population =
                    spawn_node["max_population"] ? spawn_node["max_population"].as<int>() : 0;
            zone.spawns.push_back(spawn);
        }
    }
    if (node["fixed_npcs"]) {
        for (const auto& npc_node: node["fixed_npcs"]) {
            NpcInstance npc;
            npc.type = npc_node["type"] ? npc_node["type"].as<std::string>() : std::string();
            npc.name = npc_node["name"] ? npc_node["name"].as<std::string>() : std::string();
            if (npc_node["position"] && npc_node["position"].size() >= 2) {
                npc.x = npc_node["position"][0].as<int>();
                npc.y = npc_node["position"][1].as<int>();
            }
            zone.fixed_npcs.push_back(npc);
        }
    }
    return zone;
}

Entry YamlMapIO::read_entry(const YAML::Node& node) {
    Entry entry;
    entry.id = node["id"] ? node["id"].as<std::string>() : std::string();
    entry.type = node["type"] ? node["type"].as<std::string>() : std::string();
    entry.environment_id =
            node["environment"] ? node["environment"].as<std::string>() : std::string();
    if (node["position"] && node["position"].size() >= 2) {
        entry.x = node["position"][0].as<int>();
        entry.y = node["position"][1].as<int>();
    }
    if (node["size"] && node["size"].size() >= 2) {
        entry.width = node["size"][0].as<int>();
        entry.height = node["size"][1].as<int>();
    }
    if (node["texture"]) {
        entry.texture = node["texture"].as<std::string>();
    }
    return entry;
}

Wall YamlMapIO::read_wall(const YAML::Node& node) {
    Wall wall;
    wall.id = node["id"] ? node["id"].as<std::string>() : std::string();
    wall.template_id = node["template"] ? node["template"].as<std::string>() : std::string();
    if (node["position"] && node["position"].size() >= 2) {
        wall.x = node["position"][0].as<int>();
        wall.y = node["position"][1].as<int>();
    }
    if (node["size"] && node["size"].size() >= 2) {
        wall.width = node["size"][0].as<int>();
        wall.height = node["size"][1].as<int>();
    }
    if (node["texture"]) {
        wall.texture = node["texture"].as<std::string>();
    }
    return wall;
}

Exit YamlMapIO::read_exit(const YAML::Node& node) {
    Exit exit;
    exit.id = node["id"] ? node["id"].as<std::string>() : std::string();
    exit.template_id = node["template"] ? node["template"].as<std::string>() : std::string();
    if (node["position"] && node["position"].size() >= 2) {
        exit.x = node["position"][0].as<int>();
        exit.y = node["position"][1].as<int>();
    }
    if (node["size"] && node["size"].size() >= 2) {
        exit.width = node["size"][0].as<int>();
        exit.height = node["size"][1].as<int>();
    }
    if (node["texture"]) {
        exit.texture = node["texture"].as<std::string>();
    }
    return exit;
}

Environment YamlMapIO::read_environment(const YAML::Node& node) {
    Environment env;
    env.id = node["id"] ? node["id"].as<std::string>() : std::string();
    env.name = node["name"] ? node["name"].as<std::string>() : std::string();
    env.type = node["type"] ? node["type"].as<std::string>() : std::string();
    if (node["size"] && node["size"].size() >= 2) {
        env.width = node["size"][0].as<int>();
        env.height = node["size"][1].as<int>();
    }
    if (node["player_spawn"]) {
        env.player_spawn = read_player_spawn(node["player_spawn"]);
    }
    if (node["obstacles"]) {
        for (const auto& obstacle_node: node["obstacles"]) {
            env.obstacles.push_back(read_obstacle(obstacle_node));
        }
    }
    if (node["walls"]) {
        for (const auto& wall_node: node["walls"]) {
            env.walls.push_back(read_wall(wall_node));
        }
    }
    if (node["exits"]) {
        for (const auto& exit_node: node["exits"]) {
            env.exits.push_back(read_exit(exit_node));
        }
    }
    if (node["spawns"]) {
        for (const auto& spawn_node: node["spawns"]) {
            CreatureSpawn spawn;
            spawn.creature = spawn_node["creature"] ? spawn_node["creature"].as<std::string>() :
                                                      std::string();
            spawn.max_population =
                    spawn_node["max_population"] ? spawn_node["max_population"].as<int>() : 0;
            env.spawns.push_back(spawn);
        }
    }
    if (node["floor_texture"]) {
        env.floor_texture = node["floor_texture"].as<std::string>();
    }
    return env;
}

bool YamlMapIO::load(MapDocument& document, const std::string& path) {
    try {
        const YAML::Node root = YAML::LoadFile(path);
        if (!root["map"] || !root["map"]["width"] || !root["map"]["height"]) {
            return false;
        }

        document = MapDocument();
        document.version = root["version"] ? root["version"].as<int>() : 1;

        const auto& map = root["map"];
        document.map.id = map["id"] ? map["id"].as<std::string>() : std::string();
        document.map.name = map["name"] ? map["name"].as<std::string>() : std::string();
        document.map.width = map["width"].as<int>();
        document.map.height = map["height"].as<int>();

        // El grid de tiles de piso se recalcula al guardar a partir de los biomas, 
        // pero se lee al cargar para reconstruir los tiles independientes
        if (root["biome_map"] && root["biome_map"]["data"]) {
            const int width = document.map.width;
            const int height = document.map.height;
            document.biome_grid.assign(static_cast<size_t>(width) * height, 0);
            const auto data = root["biome_map"]["data"].as<std::string>();
            int x = 0;
            int y = 0;
            for (const char c: data) {
                if (c == '\n') {
                    ++y;
                    x = 0;
                    continue;
                }
                if (x < width && y < height) {
                    const size_t idx = static_cast<size_t>(y) * width + x;
                    document.biome_grid[idx] = grid_char_to_value(c);
                }
                ++x;
            }
        }

        if (root["player_spawn"]) {
            document.player_spawn = read_player_spawn(root["player_spawn"]);
        }

        if (root["obstacles"]) {
            for (const auto& node: root["obstacles"]) {
                document.obstacles.push_back(read_obstacle(node));
            }
        }

        if (root["zones"]) {
            for (const auto& node: root["zones"]) {
                document.zones.push_back(read_zone(node));
            }
        }

        if (root["entries"]) {
            for (const auto& node: root["entries"]) {
                document.entries.push_back(read_entry(node));
            }
        }

        if (root["environments"]) {
            for (const auto& node: root["environments"]) {
                document.environments.push_back(read_environment(node));
            }
        }

        return true;
    } catch (const YAML::Exception&) {
        return false;
    }
}
