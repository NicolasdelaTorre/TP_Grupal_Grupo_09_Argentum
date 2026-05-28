#ifndef ARGENTUM_EDITOR_TOOL_INFO_H
#define ARGENTUM_EDITOR_TOOL_INFO_H

#include <QString>

enum class EditorTool {
    None,
    PlayerSpawn,
    Obstacle,
    CityZone,
    BiomeZone,
    Entry,
    Wall,
};

struct ToolInfo {
    EditorTool tool = EditorTool::None;
    QString obstacle_template_id;
    QString city_template_id;
    QString biome_template_id;
    QString entry_template_id;
    QString wall_template_id;
};

#endif
