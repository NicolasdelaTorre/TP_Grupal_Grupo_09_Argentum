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
    std::string texture;
};

struct Wall {
    std::string id;
    std::string template_id;
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;
    std::string texture;
};

struct Exit {
    std::string id;
    std::string template_id;
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;
    std::string texture;
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
    std::vector<Exit> exits;
    std::vector<CreatureSpawn> spawns;
    // Ruta relativa (a common/assets/images) de la textura de piso del entorno.
    std::string floor_texture;
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
    std::vector<Exit> exits;
    // Textura de piso para entornos (ruta relativa a common/assets/images).
    std::string floor_texture;
    std::vector<uint8_t> biome_grid;
};

#endif
