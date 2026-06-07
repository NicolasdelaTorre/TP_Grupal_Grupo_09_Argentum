#ifndef ARGENTUM_EDITOR_ASSETS_TEMPLATES_TEMPLATE_REGISTRY_H
#define ARGENTUM_EDITOR_ASSETS_TEMPLATES_TEMPLATE_REGISTRY_H

#include <string>
#include <vector>

struct NpcTemplate {
    std::string type;
    std::string name;
    int relative_x = 0;
    int relative_y = 0;
};

// Obstáculo predefinido de una ciudad. `type` referencia el template_id de un
// obstáculo; la posición es relativa a la esquina de la ciudad.
struct CityObstacleTemplate {
    std::string type;
    int relative_x = 0;
    int relative_y = 0;
};

// Modificador de piso predefinido de una ciudad. `type` referencia el template_id
// de un piso; la posición es relativa a la esquina de la ciudad.
struct CityFloorTemplate {
    std::string type;
    int relative_x = 0;
    int relative_y = 0;
};

struct CityTemplate {
    std::string id;
    std::string name;
    int default_width = 0;
    int default_height = 0;
    std::string color;
    std::vector<NpcTemplate> fixed_npcs;
    std::vector<CityObstacleTemplate> fixed_obstacles;
    std::vector<CityFloorTemplate> fixed_floors;
};

struct BiomeTemplate {
    std::string id;
    std::string name;
    int default_width = 0;
    int default_height = 0;
    std::string color;
    std::string texture;
    std::vector<std::string> allowed_creatures;
};

struct ObstacleTemplate {
    std::string id;
    std::string name;
    int width = 1;
    int height = 1;
    std::string color;
    std::string texture;
};

struct EnvironmentSizeOption {
    int width = 0;
    int height = 0;
};

struct EntryTemplate {
    std::string id;
    std::string name;
    int width = 1;
    int height = 1;
    std::string color;
    std::vector<EnvironmentSizeOption> environment_sizes;
    std::string floor_color;
};

struct WallTemplate {
    std::string id;
    std::string name;
    int width = 1;
    int height = 1;
    std::string color;
};

// Modificador de piso: una textura de 64x64 que no bloquea el paso y solo cambia
// el número de la celda en el grid (`grid_value`). Siempre ocupa 1x1.
struct FloorTemplate {
    std::string id;
    std::string name;
    std::string color;
    std::string texture;
    int grid_value = 0;
};

class TemplateRegistry {
public:
    TemplateRegistry() = default;

    bool load();
    const std::vector<CityTemplate>& cities() const;
    const std::vector<BiomeTemplate>& biomes() const;
    const std::vector<ObstacleTemplate>& obstacles() const;
    const std::vector<EntryTemplate>& entries() const;
    const std::vector<WallTemplate>& walls() const;
    const std::vector<FloorTemplate>& floors() const;

    const CityTemplate* find_city(const std::string& id) const;
    const BiomeTemplate* find_biome(const std::string& id) const;
    const ObstacleTemplate* find_obstacle(const std::string& id) const;
    const EntryTemplate* find_entry(const std::string& id) const;
    const WallTemplate* find_wall(const std::string& id) const;
    const FloorTemplate* find_floor(const std::string& id) const;
    // Modificador de piso cuyo `grid_value` coincide con el dado (para
    // reconstruir los pisos al cargar un mapa desde el grid del biome_map).
    const FloorTemplate* find_floor_by_grid_value(int grid_value) const;

private:
    std::vector<CityTemplate> cities_;
    std::vector<BiomeTemplate> biomes_;
    std::vector<ObstacleTemplate> obstacles_;
    std::vector<EntryTemplate> entries_;
    std::vector<WallTemplate> walls_;
    std::vector<FloorTemplate> floors_;

    bool load_city_file(const std::string& path);
    bool load_biome_file(const std::string& path);
    bool load_obstacle_file(const std::string& path);
    bool load_entry_file(const std::string& path);
    bool load_wall_file(const std::string& path);
    bool load_floor_file(const std::string& path);
};

#endif