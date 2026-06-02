#ifndef ARGENTUM_EDITOR_MAP_MAP_DATA_H
#define ARGENTUM_EDITOR_MAP_MAP_DATA_H

#include <cstdint>
#include <string>
#include <vector>

struct MapInfo {
    std::string id;
    std::string name;
    int width = 0;
    int height = 0;
};

struct PlayerSpawn {
    int x = 0;
    int y = 0;
    bool placed = false;
};

struct Obstacle {
    std::string id;
    std::string type;
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;
    std::string texture;
};

struct CreatureSpawn {
    std::string creature;
    int max_population = 0;
};

struct NpcInstance {
    std::string type;
    std::string name;
    int x = 0;
    int y = 0;
};

struct Zone {
    std::string id;
    std::string type;
    std::string template_id;
    int area_x = 0;
    int area_y = 0;
    int area_width = 0;
    int area_height = 0;
    std::vector<CreatureSpawn> spawns;
    std::vector<NpcInstance> fixed_npcs;
    std::string texture;
};

struct Entry {
    std::string id;
    std::string type;
    std::string environment_id;
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;
};

struct Wall {
    std::string id;
    std::string template_id;
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;
};

struct Environment {
    std::string id;
    std::string name;
    std::string type;
    int width = 0;
    int height = 0;
    PlayerSpawn player_spawn;
    std::vector<Obstacle> obstacles;
    std::vector<Wall> walls;
    std::string floor_color;
};

struct MapDocument {
    int version = 1;
    MapInfo map;
    PlayerSpawn player_spawn;
    std::vector<Obstacle> obstacles;
    std::vector<Zone> zones;
    std::vector<Entry> entries;
    std::vector<Environment> environments;
    std::vector<Wall> walls;
    std::string floor_color;
    // Grid de biomas pre-calculado (Dijkstra), row-major width*height.
    // Cada celda guarda el valor numérico de BiomeType (0 = sin bioma).
    std::vector<uint8_t> biome_grid;
};

#endif
