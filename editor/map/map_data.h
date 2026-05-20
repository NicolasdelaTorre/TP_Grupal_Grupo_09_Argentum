#ifndef ARGENTUM_EDITOR_MAP_MAP_DATA_H
#define ARGENTUM_EDITOR_MAP_MAP_DATA_H

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
};

struct MapDocument {
    int version = 1;
    MapInfo map;
    PlayerSpawn player_spawn;
    std::vector<Obstacle> obstacles;
    std::vector<Zone> zones;
};

#endif
