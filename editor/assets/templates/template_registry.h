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

struct CityTemplate {
    std::string id;
    std::string name;
    int default_width = 0;
    int default_height = 0;
    std::vector<NpcTemplate> fixed_npcs;
};

struct ForestTemplate {
    std::string id;
    std::string name;
    int default_width = 0;
    int default_height = 0;
    std::vector<std::string> allowed_creatures;
};

class TemplateRegistry {
public:
    TemplateRegistry() = default;

    bool load();
    const std::vector<CityTemplate>& cities() const;
    const std::vector<ForestTemplate>& forests() const;

    const CityTemplate* find_city(const std::string& id) const;
    const ForestTemplate* find_forest(const std::string& id) const;

private:
    std::vector<CityTemplate> cities_;
    std::vector<ForestTemplate> forests_;

    bool load_city_file(const std::string& path);
    bool load_forest_file(const std::string& path);
};

#endif
