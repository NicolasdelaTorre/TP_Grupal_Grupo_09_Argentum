#include "scene_controller.h"

#include <algorithm>
#include <filesystem>

#include "editor_constants.h"

SceneController::SceneController(QGraphicsScene* scene, const TemplateRegistry& templates):
        scene_(scene), templates_(templates) {}

QColor SceneController::resolveZoneColor(const std::string& template_color, bool is_city) const {
    if (!template_color.empty()) {
        QColor parsed(QString::fromStdString(template_color));
        if (parsed.isValid()) {
            return parsed;
        }
    }
    return is_city ? QColor(180, 220, 255) : QColor(80, 160, 80);
}

void SceneController::reset() {
    biome_spawns_.clear();
    next_obstacle_id_ = 1;
    next_zone_id_ = 1;
    next_wall_id_ = 1;
}

QString SceneController::nextObstacleId() {
    return QStringLiteral("obstacle_%1").arg(next_obstacle_id_++);
}

QString SceneController::nextZoneId() { return QStringLiteral("zone_%1").arg(next_zone_id_++); }

QString SceneController::nextWallId() {
    return QStringLiteral("wall_%1").arg(next_wall_id_++);
}

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
    item->setZValue(Z_PLAYER_SPAWN);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeObstacle(const ToolInfo& tool, int cell_x, int cell_y, QString& error) {
    if (tool.obstacle_template_id.isEmpty()) {
        error = QStringLiteral("Seleccioná un template de obstáculo.");
        return false;
    }

    const auto* obstacle = templates_.find_obstacle(tool.obstacle_template_id.toStdString());
    if (!obstacle) {
        error = QStringLiteral("Template de obstáculo inválido.");
        return false;
    }

    QColor fill(120, 90, 60);
    if (!obstacle->color.empty()) {
        QColor parsed(QString::fromStdString(obstacle->color));
        if (parsed.isValid()) {
            fill = parsed;
        }
    }

    const QString id = nextObstacleId();
    auto* item = item_builder_.buildObstacle(id, QString::fromStdString(obstacle->id),
                                             obstacle->width, obstacle->height, fill,
                                             QString::fromStdString(obstacle->texture));
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_OBSTACLE);
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

    const auto* city = templates_.find_city(tool.city_template_id.toStdString());
    const QColor fill = resolveZoneColor(city ? city->color : std::string(), true);

    const QString id = zone_id.isEmpty() ? nextZoneId() : zone_id;
    auto* item = item_builder_.buildZone(id, ZONE_TYPE_CITY, tool.city_template_id, width, height,
                                         fill);
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_CITY_ZONE);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeEntry(const QString& entry_id, const QString& environment_id,
                                  const QString& template_id, int cell_x, int cell_y,
                                  QString& error) {
    if (template_id.isEmpty()) {
        error = QStringLiteral("Seleccioná un template de entrada.");
        return false;
    }

    const auto* tpl = templates_.find_entry(template_id.toStdString());
    if (!tpl) {
        error = QStringLiteral("Template de entrada inválido.");
        return false;
    }

    QColor fill(120, 90, 60);
    if (!tpl->color.empty()) {
        QColor parsed(QString::fromStdString(tpl->color));
        if (parsed.isValid()) {
            fill = parsed;
        }
    }

    auto* item = item_builder_.buildEntry(entry_id, QString::fromStdString(tpl->id),
                                          environment_id, tpl->width, tpl->height, fill);
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_ENTRY);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeWall(const ToolInfo& tool, int cell_x, int cell_y, QString& error,
                                const QString& wall_id) {
    if (tool.wall_template_id.isEmpty()) {
        error = QStringLiteral("Seleccioná un template de pared.");
        return false;
    }

    const auto* wall_tpl = templates_.find_wall(tool.wall_template_id.toStdString());
    if (!wall_tpl) {
        error = QStringLiteral("Template de pared inválido.");
        return false;
    }

    QColor fill(120, 120, 120);
    if (!wall_tpl->color.empty()) {
        QColor parsed(QString::fromStdString(wall_tpl->color));
        if (parsed.isValid()) {
            fill = parsed;
        }
    }

    const QString id = wall_id.isEmpty() ? nextWallId() : wall_id;
    auto* item = item_builder_.buildWall(id, QString::fromStdString(wall_tpl->id), wall_tpl->width,
                                         wall_tpl->height, fill);
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_WALL);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeBiomeZone(const ToolInfo& tool, int cell_x, int cell_y, int width,
                                      int height,
                                      const std::vector<CreatureSpawn>& spawns,
                                      QString& error, const QString& zone_id) {
    if (tool.biome_template_id.isEmpty()) {
        error = QStringLiteral("Seleccioná un template de bioma.");
        return false;
    }
    if (width <= 0 || height <= 0) {
        error = QStringLiteral("El área de la zona es inválida.");
        return false;
    }

    const auto* biome = templates_.find_biome(tool.biome_template_id.toStdString());
    const QColor fill = resolveZoneColor(biome ? biome->color : std::string(), false);

    const QString id = zone_id.isEmpty() ? nextZoneId() : zone_id;
    auto* item = item_builder_.buildZone(id, ZONE_TYPE_BIOME, tool.biome_template_id, width,
                                       height, fill);
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_BIOME_ZONE);
    scene_->addItem(item);
    biome_spawns_.insert(id, spawns);
    return true;
}

// eliminar item en celda
DeletedItem SceneController::deleteAtCell(int cell_x, int cell_y) {
    DeletedItem result;
    auto* item = topLevelItemAtCell(cell_x, cell_y);
    if (!item) {
        return result;
    }

    const QString item_id = item->data(DATA_ID).toString();
    const QString item_type = item->data(DATA_TYPE).toString();

    result.deleted = true;
    result.type = item_type;
    result.id = item_id;

    if (item_type == BIOME_ZONE_TYPE) {
        biome_spawns_.remove(item_id);
    } else if (item_type == ENTRY_TYPE) {
        result.environment_id = item->data(DATA_ENVIRONMENT_ID).toString();
    }

    scene_->removeItem(item);
    delete item;
    return result;
}

const QHash<QString, std::vector<CreatureSpawn>>& SceneController::biome_spawns() const {
    return biome_spawns_;
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
            // Resolver textura via template
            if (const auto* tpl = templates_.find_obstacle(obstacle.type)) {
                if (!tpl->texture.empty()) {
                    obstacle.texture =
                            std::filesystem::path(tpl->texture).filename().string();
                }
            }
            document.obstacles.push_back(obstacle);
            continue;
        }

        if (type == ENTRY_TYPE) {
            Entry entry;
            entry.id = item->data(DATA_ID).toString().toStdString();
            entry.type = item->data(DATA_SUBTYPE).toString().toStdString();
            entry.environment_id = item->data(DATA_ENVIRONMENT_ID).toString().toStdString();
            entry.x = cell_x;
            entry.y = cell_y;
            entry.width = item->data(DATA_WIDTH).toInt();
            entry.height = item->data(DATA_HEIGHT).toInt();
            document.entries.push_back(entry);
            continue;
        }

        if (type == WALL_TYPE) {
            Wall wall;
            wall.id = item->data(DATA_ID).toString().toStdString();
            wall.template_id = item->data(DATA_SUBTYPE).toString().toStdString();
            wall.x = cell_x;
            wall.y = cell_y;
            wall.width = item->data(DATA_WIDTH).toInt();
            wall.height = item->data(DATA_HEIGHT).toInt();
            document.walls.push_back(wall);
            continue;
        }

        if (type == CITY_ZONE_TYPE || type == BIOME_ZONE_TYPE) {
            Zone zone;
            zone.id = item->data(DATA_ID).toString().toStdString();
            zone.type = (type == CITY_ZONE_TYPE) ? ZONE_TYPE_CITY : ZONE_TYPE_BIOME;
            zone.template_id = item->data(DATA_SUBTYPE).toString().toStdString();
            zone.area_x = cell_x;
            zone.area_y = cell_y;
            zone.area_width = item->data(DATA_WIDTH).toInt();
            zone.area_height = item->data(DATA_HEIGHT).toInt();

            if (type == BIOME_ZONE_TYPE) {
                const auto it = biome_spawns_.find(item->data(DATA_ID).toString());
                if (it != biome_spawns_.end()) {
                    zone.spawns = it.value();
                }
                
                if (const auto* tpl = templates_.find_biome(zone.template_id)) {
                    if (!tpl->texture.empty()) {
                        zone.texture =
                                std::filesystem::path(tpl->texture).filename().string();
                    }
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