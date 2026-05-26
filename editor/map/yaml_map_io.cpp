#include "yaml_map_io.h"

#include <fstream>

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

        if (document.player_spawn.placed) {
            out << YAML::Key << "player_spawn" << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "position" << YAML::Value;
            out << YAML::Flow << YAML::BeginSeq << document.player_spawn.x << document.player_spawn.y << YAML::EndSeq;
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
                out << YAML::Flow << YAML::BeginSeq << obstacle.width << obstacle.height << YAML::EndSeq;
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
                out << YAML::Flow << YAML::BeginSeq << entry.width << entry.height
                    << YAML::EndSeq;
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
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;
                }

                if (!env.floor_color.empty()) {
                    out << YAML::Key << "floor_color" << YAML::Value << env.floor_color;
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
