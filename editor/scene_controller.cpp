#include "scene_controller.h"

#include <algorithm>

#include "editor_constants.h"

SceneController::SceneController(QGraphicsScene* scene, const TemplateRegistry& templates):
        scene_(scene), templates_(templates) {}

void SceneController::reset() {
    forest_spawns_.clear();
    next_obstacle_id_ = 1;
    next_zone_id_ = 1;
}

QString SceneController::nextObstacleId() {
    return QStringLiteral("obstacle_%1").arg(next_obstacle_id_++);
}

QString SceneController::nextZoneId() { return QStringLiteral("zone_%1").arg(next_zone_id_++); }

// obtener item en celda
QGraphicsItem* SceneController::topLevelItemAtCell(int cell_x, int cell_y) const {
    const QRectF area(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE, CELL_DISPLAY_SIZE,
                      CELL_DISPLAY_SIZE);
    const auto items = scene_->items(area);
    for (auto* item: items) {
        QGraphicsItem* current = item;
        while (current->parentItem()) {
            current = current->parentItem();
        }
        if (current->data(DATA_TYPE).isValid()) {
            return current;
        }
    }
    return nullptr;
}

bool SceneController::placePlayerSpawn(int cell_x, int cell_y, QString& error) {
    Q_UNUSED(error);
    auto* item = item_builder_.buildPlayerSpawn(QStringLiteral("player_spawn"));
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeObstacle(const ToolInfo& tool, int cell_x, int cell_y, QString& error) {
    if (tool.obstacle_width <= 0 || tool.obstacle_height <= 0) {
        error = QStringLiteral("El tamaño del obstáculo debe ser mayor a cero.");
        return false;
    }

    const QString id = nextObstacleId();
    auto* item = item_builder_.buildObstacle(id, tool.obstacle_type, tool.obstacle_width,
                                             tool.obstacle_height);
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(2);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeCityZone(const ToolInfo& tool, int cell_x, int cell_y, int width,
                                    int height, QString& error, const QString& zone_id) {
    if (tool.city_template_id.isEmpty()) {
        error = QStringLiteral("Seleccioná un template de ciudad.");
        return false;
    }
    if (width <= 0 || height <= 0) {
        error = QStringLiteral("El área de la zona es inválida.");
        return false;
    }

    const QString id = zone_id.isEmpty() ? nextZoneId() : zone_id;
    auto* item = item_builder_.buildZone(id, ZONE_TYPE_CITY, tool.city_template_id, width, height);
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(1);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeForestZone(const ToolInfo& tool, int cell_x, int cell_y, int width,
                                      int height, const std::vector<CreatureSpawn>& spawns,
                                      QString& error, const QString& zone_id) {
    if (tool.forest_template_id.isEmpty()) {
        error = QStringLiteral("Seleccioná un template de bosque.");
        return false;
    }
    if (spawns.empty()) {
        error = QStringLiteral("Seleccioná al menos una criatura con cantidad mayor a cero.");
        return false;
    }
    if (width <= 0 || height <= 0) {
        error = QStringLiteral("El área de la zona es inválida.");
        return false;
    }

    const QString id = zone_id.isEmpty() ? nextZoneId() : zone_id;
    auto* item =
            item_builder_.buildZone(id, ZONE_TYPE_FOREST, tool.forest_template_id, width, height);
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(1);
    scene_->addItem(item);
    forest_spawns_.insert(id, spawns);
    return true;
}

// eliminar item en celda
void SceneController::deleteAtCell(int cell_x, int cell_y) {
    auto* item = topLevelItemAtCell(cell_x, cell_y);
    if (!item) {
        return;
    }

    const QString zone_id = item->data(DATA_ID).toString();
    if (item->data(DATA_TYPE).toString() == FOREST_ZONE_TYPE) {
        forest_spawns_.remove(zone_id);
    }

    scene_->removeItem(item);
    delete item;
}

const QHash<QString, std::vector<CreatureSpawn>>& SceneController::forest_spawns() const {
    return forest_spawns_;
}

// construir documento con datos del mapa
MapDocument SceneController::buildDocument(const QString& map_id, const QString& map_name,
                                           int width, int height) const {
    MapDocument document;
    document.version = 1;
    document.map.id = map_id.toStdString();
    document.map.name = map_name.toStdString();
    document.map.width = width;
    document.map.height = height;

    for (auto* item: scene_->items()) {
        const QString type = item->data(DATA_TYPE).toString();
        if (type.isEmpty()) {
            continue;
        }

        const int cell_x = static_cast<int>(item->pos().x()) / CELL_DISPLAY_SIZE;
        const int cell_y = static_cast<int>(item->pos().y()) / CELL_DISPLAY_SIZE;

        if (type == PLAYER_SPAWN_TYPE) {
            document.player_spawn.x = cell_x;
            document.player_spawn.y = cell_y;
            document.player_spawn.placed = true;
            continue;
        }

        if (type == OBSTACLE_TYPE) {
            Obstacle obstacle;
            obstacle.id = item->data(DATA_ID).toString().toStdString();
            obstacle.type = item->data(DATA_SUBTYPE).toString().toStdString();
            obstacle.x = cell_x;
            obstacle.y = cell_y;
            obstacle.width = item->data(DATA_WIDTH).toInt();
            obstacle.height = item->data(DATA_HEIGHT).toInt();
            document.obstacles.push_back(obstacle);
            continue;
        }

        if (type == CITY_ZONE_TYPE || type == FOREST_ZONE_TYPE) {
            Zone zone;
            zone.id = item->data(DATA_ID).toString().toStdString();
            zone.type = (type == CITY_ZONE_TYPE) ? ZONE_TYPE_CITY : ZONE_TYPE_FOREST;
            zone.template_id = item->data(DATA_SUBTYPE).toString().toStdString();
            zone.area_x = cell_x;
            zone.area_y = cell_y;
            zone.area_width = item->data(DATA_WIDTH).toInt();
            zone.area_height = item->data(DATA_HEIGHT).toInt();

            if (type == FOREST_ZONE_TYPE) {
                const auto it = forest_spawns_.find(item->data(DATA_ID).toString());
                if (it != forest_spawns_.end()) {
                    zone.spawns = it.value();
                }
            }

            if (type == CITY_ZONE_TYPE) {
                if (const auto* city = templates_.find_city(zone.template_id)) {
                    for (const auto& npc_template: city->fixed_npcs) {
                        NpcInstance npc;
                        npc.type = npc_template.type;
                        npc.name = npc_template.name;
                        npc.x = zone.area_x + npc_template.relative_x;
                        npc.y = zone.area_y + npc_template.relative_y;
                        zone.fixed_npcs.push_back(npc);
                    }
                }
            }

            document.zones.push_back(zone);
        }
    }

    return document;
}
