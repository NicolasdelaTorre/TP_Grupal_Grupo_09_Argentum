#ifndef ARGENTUM_EDITOR_MAP_YAML_MAP_IO_H
#define ARGENTUM_EDITOR_MAP_YAML_MAP_IO_H

#include <string>

#include <yaml-cpp/yaml.h>

#include "map_data.h"

class YamlMapIO {
public:
    static bool save(const MapDocument& document, const std::string& path);

    // EReconstruye el MapDocument completo para poder seguir editándolo.
    static bool load(MapDocument& document, const std::string& path);

private:
    static void write_position(YAML::Emitter& out, int x, int y);
    static void write_size(YAML::Emitter& out, int width, int height);
    static void write_floor_grid(YAML::Emitter& out, const MapDocument& document);

    static PlayerSpawn read_player_spawn(const YAML::Node& node);
    static Obstacle read_obstacle(const YAML::Node& node);
    static Zone read_zone(const YAML::Node& node);
    static Entry read_entry(const YAML::Node& node);
    static Wall read_wall(const YAML::Node& node);
    static Exit read_exit(const YAML::Node& node);
    static Environment read_environment(const YAML::Node& node);
};

#endif
