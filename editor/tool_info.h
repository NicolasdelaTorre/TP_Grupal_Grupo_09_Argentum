#ifndef ARGENTUM_EDITOR_TOOL_INFO_H
#define ARGENTUM_EDITOR_TOOL_INFO_H

#include <QString>

enum class EditorTool {
    None,
    PlayerSpawn,
    Obstacle,
    CityZone,
    ForestZone,
};

struct ToolInfo {
    EditorTool tool = EditorTool::None;
    QString obstacle_type = QStringLiteral("tree");
    int obstacle_width = 1;
    int obstacle_height = 1;
    QString city_template_id;
    QString forest_template_id;
};

#endif
