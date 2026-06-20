#include "scene_controller.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <string>

#include "common_biome.h"
#include "map/biome_grid.h"

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
    next_exit_id_ = 1;
    next_floor_id_ = 1;
}

QString SceneController::nextObstacleId() {
    return QStringLiteral("obstacle_%1").arg(next_obstacle_id_++);
}

QString SceneController::nextZoneId() { return QStringLiteral("zone_%1").arg(next_zone_id_++); }

QString SceneController::nextWallId() { return QStringLiteral("wall_%1").arg(next_wall_id_++); }

QString SceneController::nextExitId() { return QStringLiteral("exit_%1").arg(next_exit_id_++); }

QString SceneController::nextFloorId() { return QStringLiteral("floor_%1").arg(next_floor_id_++); }

void SceneController::bumpCounter(int& counter, const QString& id, const QString& prefix) {
    if (!id.startsWith(prefix)) {
        return;
    }
    bool ok = false;
    const int value = id.mid(prefix.size()).toInt(&ok);
    if (ok && value + 1 > counter) {
        counter = value + 1;
    }
}

// obtener item superior en celda
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

void SceneController::removeItemsOfType(const QString& type) {
    std::vector<QGraphicsItem*> to_delete;
    for (auto* item: scene_->items()) {
        QGraphicsItem* current = item;
        while (current->parentItem()) {
            current = current->parentItem();
        }
        if (current->data(DATA_TYPE).toString() != type) {
            continue;
        }
        if (std::find(to_delete.begin(), to_delete.end(), current) == to_delete.end()) {
            to_delete.push_back(current);
        }
    }
    for (auto* item: to_delete) {
        scene_->removeItem(item);
        delete item;
    }
}

namespace {

bool blocksPlayerSpawn(const QString& type) {
    return type == OBSTACLE_TYPE || type == WALL_TYPE || type == EXIT_TYPE || type == ENTRY_TYPE;
}

}  // namespace

bool SceneController::placePlayerSpawn(int cell_x, int cell_y, QString& error,
                                       bool validate_position) {
    if (validate_position) {
        const QString existing = itemTypeAtCell(cell_x, cell_y);
        if (blocksPlayerSpawn(existing)) {
            error = QStringLiteral(
                    "Player spawn cannot be placed over an obstacle, wall, exit, or entry.");
            return false;
        }
    }
    removeItemsOfType(PLAYER_SPAWN_TYPE);
    auto* item = item_builder_.buildPlayerSpawn(QStringLiteral("player_spawn"));
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_PLAYER_SPAWN);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeObstacle(const ToolInfo& tool, int cell_x, int cell_y, QString& error,
                                    int texture_anchor_override) {
    if (tool.obstacle_template_id.isEmpty()) {
        error = QStringLiteral("Select an obstacle template.");
        return false;
    }

    const auto* obstacle = templates_.find_obstacle(tool.obstacle_template_id.toStdString());
    if (!obstacle) {
        error = QStringLiteral("Invalid obstacle template.");
        return false;
    }

    const int texture_anchor =
            texture_anchor_override >= 0 ? texture_anchor_override : obstacle->texture_anchor;

    const QString id = nextObstacleId();
    auto* item = item_builder_.buildObstacle(id, QString::fromStdString(obstacle->id),
                                             obstacle->width, obstacle->height,
                                             QString::fromStdString(obstacle->texture),
                                             texture_anchor);
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_OBSTACLE);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeCityZone(const ToolInfo& tool, int cell_x, int cell_y, int width,
                                    int height, QString& error, const QString& zone_id) {
    if (tool.city_template_id.isEmpty()) {
        error = QStringLiteral("Select a city template.");
        return false;
    }
    if (width <= 0 || height <= 0) {
        error = QStringLiteral("The zone area is invalid.");
        return false;
    }

    const auto* city = templates_.find_city(tool.city_template_id.toStdString());
    const QColor fill = resolveZoneColor(city ? city->color : std::string(), true);

    const QString id = zone_id.isEmpty() ? nextZoneId() : zone_id;
    bumpCounter(next_zone_id_, id, QStringLiteral("zone_"));
    auto* item =
            item_builder_.buildZone(id, ZONE_TYPE_CITY, tool.city_template_id, width, height, fill);
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_CITY_ZONE);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeEntry(const QString& entry_id, const QString& environment_id,
                                 const QString& template_id, int cell_x, int cell_y,
                                 QString& error) {
    if (template_id.isEmpty()) {
        error = QStringLiteral("Select an entry template.");
        return false;
    }

    const auto* tpl = templates_.find_entry(template_id.toStdString());
    if (!tpl) {
        error = QStringLiteral("Invalid entry template.");
        return false;
    }

    auto* item = item_builder_.buildEntry(entry_id, QString::fromStdString(tpl->id), environment_id,
                                          tpl->width, tpl->height,
                                          QString::fromStdString(tpl->texture));
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_ENTRY);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeWall(const ToolInfo& tool, int cell_x, int cell_y, QString& error,
                                const QString& wall_id) {
    if (tool.wall_template_id.isEmpty()) {
        error = QStringLiteral("Select a wall template.");
        return false;
    }

    const auto* wall_tpl = templates_.find_wall(tool.wall_template_id.toStdString());
    if (!wall_tpl) {
        error = QStringLiteral("Invalid wall template.");
        return false;
    }

    const QString id = wall_id.isEmpty() ? nextWallId() : wall_id;
    bumpCounter(next_wall_id_, id, QStringLiteral("wall_"));
    auto* item = item_builder_.buildWall(id, QString::fromStdString(wall_tpl->id), wall_tpl->width,
                                         wall_tpl->height,
                                         QString::fromStdString(wall_tpl->texture));
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_WALL);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeExit(const ToolInfo& tool, int cell_x, int cell_y, QString& error,
                                const QString& exit_id) {
    if (tool.exit_template_id.isEmpty()) {
        error = QStringLiteral("Select an exit template.");
        return false;
    }

    const auto* exit_tpl = templates_.find_exit(tool.exit_template_id.toStdString());
    if (!exit_tpl) {
        error = QStringLiteral("Invalid exit template.");
        return false;
    }

    const QString id = exit_id.isEmpty() ? nextExitId() : exit_id;
    bumpCounter(next_exit_id_, id, QStringLiteral("exit_"));
    auto* item = item_builder_.buildExit(id, QString::fromStdString(exit_tpl->id), exit_tpl->width,
                                         exit_tpl->height,
                                         QString::fromStdString(exit_tpl->texture));
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_EXIT);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeFloor(const ToolInfo& tool, int cell_x, int cell_y, QString& error,
                                 const QString& floor_id) {
    if (tool.floor_template_id.isEmpty()) {
        error = QStringLiteral("Select a floor modifier.");
        return false;
    }

    const auto* floor = templates_.find_floor(tool.floor_template_id.toStdString());
    if (!floor) {
        error = QStringLiteral("Invalid floor modifier.");
        return false;
    }

    // si ya hay uno, se reemplaza.
    const QRectF area(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE, CELL_DISPLAY_SIZE,
                      CELL_DISPLAY_SIZE);
    std::vector<QGraphicsItem*> floors_to_delete;
    for (auto* existing: scene_->items(area)) {
        QGraphicsItem* current = existing;
        while (current->parentItem()) {
            current = current->parentItem();
        }
        if (current->data(DATA_TYPE).toString() == FLOOR_TYPE) {
            const int ex = static_cast<int>(current->pos().x()) / CELL_DISPLAY_SIZE;
            const int ey = static_cast<int>(current->pos().y()) / CELL_DISPLAY_SIZE;
            if (ex == cell_x && ey == cell_y) {
                if (std::find(floors_to_delete.begin(), floors_to_delete.end(), current) ==
                    floors_to_delete.end()) {
                    floors_to_delete.push_back(current);
                }
            }
        }
    }
    for (auto* floor_item: floors_to_delete) {
        scene_->removeItem(floor_item);
        delete floor_item;
    }

    const QString id = floor_id.isEmpty() ? nextFloorId() : floor_id;
    bumpCounter(next_floor_id_, id, QStringLiteral("floor_"));
    auto* item = item_builder_.buildFloor(id, QString::fromStdString(floor->id),
                                          QString::fromStdString(floor->texture));
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_FLOOR_MODIFIER);
    scene_->addItem(item);
    return true;
}

bool SceneController::placeBiomeZone(const ToolInfo& tool, int cell_x, int cell_y, int width,
                                     int height, const std::vector<CreatureSpawn>& spawns,
                                     QString& error, const QString& zone_id) {
    if (tool.biome_template_id.isEmpty()) {
        error = QStringLiteral("Select a biome template.");
        return false;
    }
    if (width <= 0 || height <= 0) {
        error = QStringLiteral("The zone area is invalid.");
        return false;
    }

    // el overlay translúcido de la zona
    const auto* biome_tpl = templates_.find_biome(tool.biome_template_id.toStdString());
    const QColor fill =
            resolveZoneColor(biome_tpl ? biome_tpl->color : std::string(), false);

    const QString id = zone_id.isEmpty() ? nextZoneId() : zone_id;
    bumpCounter(next_zone_id_, id, QStringLiteral("zone_"));
    auto* item = item_builder_.buildZone(id, ZONE_TYPE_BIOME, tool.biome_template_id, width, height,
                                         fill);
    item->setPos(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE);
    item->setZValue(Z_BIOME_ZONE);
    scene_->addItem(item);
    biome_spawns_.insert(id, spawns);
    return true;
}

QString SceneController::itemTypeAtCell(int cell_x, int cell_y) const {
    auto* item = topLevelItemAtCell(cell_x, cell_y);
    return item ? item->data(DATA_TYPE).toString() : QString();
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

    int city_x = 0;
    int city_y = 0;
    int city_w = 0;
    int city_h = 0;
    if (item_type == BIOME_ZONE_TYPE) {
        biome_spawns_.remove(item_id);
    } else if (item_type == ENTRY_TYPE) {
        result.environment_id = item->data(DATA_ENVIRONMENT_ID).toString();
    } else if (item_type == CITY_ZONE_TYPE) {
        city_x = static_cast<int>(item->pos().x()) / CELL_DISPLAY_SIZE;
        city_y = static_cast<int>(item->pos().y()) / CELL_DISPLAY_SIZE;
        city_w = item->data(DATA_WIDTH).toInt();
        city_h = item->data(DATA_HEIGHT).toInt();
    }

    scene_->removeItem(item);
    delete item;

    // al borrar una ciudad, borramos todos que esta dentro
    if (item_type == CITY_ZONE_TYPE) {
        deleteItemsInArea(OBSTACLE_TYPE, city_x, city_y, city_w, city_h);
        deleteItemsInArea(FLOOR_TYPE, city_x, city_y, city_w, city_h);
    }
    return result;
}

void SceneController::deleteItemsInArea(const QString& type, int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) {
        return;
    }
    std::vector<QGraphicsItem*> to_delete;
    for (auto* item: scene_->items()) {
        QGraphicsItem* current = item;
        while (current->parentItem()) {
            current = current->parentItem();
        }
        if (current->data(DATA_TYPE).toString() != type) {
            continue;
        }
        const int cell_x = static_cast<int>(current->pos().x()) / CELL_DISPLAY_SIZE;
        const int cell_y = static_cast<int>(current->pos().y()) / CELL_DISPLAY_SIZE;
        if (cell_x >= x && cell_x < x + w && cell_y >= y && cell_y < y + h) {
            if (std::find(to_delete.begin(), to_delete.end(), current) == to_delete.end()) {
                to_delete.push_back(current);
            }
        }
    }
    for (auto* item: to_delete) {
        scene_->removeItem(item);
        delete item;
    }
}

std::vector<CreatureSpawn> SceneController::biomeSpawnsFor(const QString& zone_id) const {
    const auto it = biome_spawns_.find(zone_id);
    return it == biome_spawns_.end() ? std::vector<CreatureSpawn>() : it.value();
}

void SceneController::setBiomeSpawns(const QString& zone_id,
                                     const std::vector<CreatureSpawn>& spawns) {
    biome_spawns_.insert(zone_id, spawns);
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

    // fuentes de bioma para reconstruir el grid con Dijkstra
    std::vector<BiomeSource> biome_sources;
    std::vector<uint8_t> biome_values;

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
            obstacle.texture_anchor = item->data(DATA_TEXTURE_ANCHOR).toInt();
            // resolver textura via template
            if (const auto* tpl = templates_.find_obstacle(obstacle.type)) {
                if (!tpl->texture.empty()) {
                    obstacle.texture = std::filesystem::path(tpl->texture).filename().string();
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
            if (const auto* tpl = templates_.find_entry(entry.type)) {
                if (!tpl->texture.empty()) {
                    entry.texture = std::filesystem::path(tpl->texture).filename().string();
                }
            }
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
            if (const auto* tpl = templates_.find_wall(wall.template_id)) {
                if (!tpl->texture.empty()) {
                    wall.texture = std::filesystem::path(tpl->texture).filename().string();
                }
            }
            document.walls.push_back(wall);
            continue;
        }

        if (type == EXIT_TYPE) {
            Exit exit;
            exit.id = item->data(DATA_ID).toString().toStdString();
            exit.template_id = item->data(DATA_SUBTYPE).toString().toStdString();
            exit.x = cell_x;
            exit.y = cell_y;
            exit.width = item->data(DATA_WIDTH).toInt();
            exit.height = item->data(DATA_HEIGHT).toInt();
            if (const auto* tpl = templates_.find_exit(exit.template_id)) {
                if (!tpl->texture.empty()) {
                    exit.texture = std::filesystem::path(tpl->texture).filename().string();
                }
            }
            document.exits.push_back(exit);
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
                        zone.texture = std::filesystem::path(tpl->texture).filename().string();
                    }
                }

                biome_sources.push_back(
                        {zone.area_x, zone.area_y, zone.area_width, zone.area_height});
                biome_values.push_back(
                        static_cast<uint8_t>(biome_from_template_id(zone.template_id)));
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


    document.biome_grid.assign(static_cast<size_t>(width) * height,
                               static_cast<uint8_t>(BiomeType::NONE));
    if (!biome_sources.empty()) {
        const std::vector<int> owners = computeBiomeOwners(width, height, biome_sources);
        for (size_t i = 0; i < owners.size(); ++i) {
            const int o = owners[i];
            if (o >= 0) {
                document.biome_grid[i] = biome_values[static_cast<size_t>(o)];
            }
        }
    }

    // los floors se superponen al grid de biomas
    for (auto* item: scene_->items()) {
        if (item->data(DATA_TYPE).toString() != FLOOR_TYPE) {
            continue;
        }
        const int cell_x = static_cast<int>(item->pos().x()) / CELL_DISPLAY_SIZE;
        const int cell_y = static_cast<int>(item->pos().y()) / CELL_DISPLAY_SIZE;
        if (cell_x < 0 || cell_y < 0 || cell_x >= width || cell_y >= height) {
            continue;
        }
        const auto* floor = templates_.find_floor(item->data(DATA_SUBTYPE).toString().toStdString());
        if (!floor) {
            continue;
        }
        const size_t idx = static_cast<size_t>(cell_y) * width + cell_x;
        document.biome_grid[idx] = static_cast<uint8_t>(floor->grid_value);
    }

    return document;
}
