#ifndef ARGENTUM_EDITOR_MAP_YAML_MAP_IO_H
#define ARGENTUM_EDITOR_MAP_YAML_MAP_IO_H

#include <string>

#include <yaml-cpp/yaml.h>

#include "map_data.h"

class YamlMapIO {
public:
    static bool save(const MapDocument& document, const std::string& path);

private:
    static void write_position(YAML::Emitter& out, int x, int y);
    static void write_size(YAML::Emitter& out, int width, int height);
};

#endif
