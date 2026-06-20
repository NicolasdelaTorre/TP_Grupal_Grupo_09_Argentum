#ifndef ARGENTUM_EDITOR_SCENE_CONTROLLER_H
#define ARGENTUM_EDITOR_SCENE_CONTROLLER_H

#include <QColor>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QHash>
#include <QString>
#include <string>
#include <vector>

#include "assets/templates/template_registry.h"
#include "map/map_data.h"

#include "item_builder.h"
#include "tool_info.h"

struct DeletedItem {
    bool deleted = false;
    QString type;
    QString id;
    QString environment_id;
};

class SceneController {
public:
    SceneController(QGraphicsScene* scene, const TemplateRegistry& templates);

    bool placePlayerSpawn(int cell_x, int cell_y, QString& error, bool validate_position = true);
    bool placeObstacle(const ToolInfo& tool, int cell_x, int cell_y, QString& error,
                       int texture_anchor_override = -1);
    bool placeCityZone(const ToolInfo& tool, int cell_x, int cell_y, int width, int height,
                       QString& error, const QString& zone_id = QString());
    bool placeBiomeZone(const ToolInfo& tool, int cell_x, int cell_y, int width, int height,
                        const std::vector<CreatureSpawn>& spawns, QString& error,
                        const QString& zone_id = QString());
    bool placeEntry(const QString& entry_id, const QString& environment_id,
                    const QString& template_id, int cell_x, int cell_y, QString& error);
    bool placeWall(const ToolInfo& tool, int cell_x, int cell_y, QString& error,
                   const QString& wall_id = QString());
    bool placeExit(const ToolInfo& tool, int cell_x, int cell_y, QString& error,
                   const QString& exit_id = QString());
    bool placeFloor(const ToolInfo& tool, int cell_x, int cell_y, QString& error,
                    const QString& floor_id = QString());

    DeletedItem deleteAtCell(int cell_x, int cell_y);
    QString itemTypeAtCell(int cell_x, int cell_y) const;

    MapDocument buildDocument(const QString& map_id, const QString& map_name, int width,
                              int height) const;

    void reset();

    std::vector<CreatureSpawn> biomeSpawnsFor(const QString& zone_id) const;
    void setBiomeSpawns(const QString& zone_id, const std::vector<CreatureSpawn>& spawns);

private:
    QGraphicsScene* scene_;
    const TemplateRegistry& templates_;
    ItemBuilder item_builder_;
    int next_obstacle_id_ = 1;
    int next_zone_id_ = 1;
    int next_wall_id_ = 1;
    int next_exit_id_ = 1;
    int next_floor_id_ = 1;
    QHash<QString, std::vector<CreatureSpawn>> biome_spawns_;

    QString nextObstacleId();
    QString nextZoneId();
    QString nextWallId();
    QString nextExitId();
    QString nextFloorId();

    static void bumpCounter(int& counter, const QString& id, const QString& prefix);
    QGraphicsItem* topLevelItemAtCell(int cell_x, int cell_y) const;
    QColor resolveZoneColor(const std::string& template_color, bool is_city) const;
    void removeItemsOfType(const QString& type);
    void deleteItemsInArea(const QString& type, int x, int y, int w, int h);
};

#endif
