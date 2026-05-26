#include "item_builder.h"

#include <QPen>

#include "editor_constants.h"

QGraphicsRectItem* ItemBuilder::make_rect(const QColor& fill, const QColor& border) {
    auto* rect = new QGraphicsRectItem(0, 0, CELL_DISPLAY_SIZE, CELL_DISPLAY_SIZE);
    rect->setBrush(QBrush(fill));
    rect->setPen(QPen(border, 1));
    return rect;
}

void ItemBuilder::attachLabel(QGraphicsRectItem* rect, const QString& text) {
    auto* label = new QGraphicsSimpleTextItem(text, rect);
    label->setPos(2, 2);
}

QGraphicsRectItem* ItemBuilder::buildPlayerSpawn(const QString& id) {
    auto* rect = make_rect(QColor(80, 180, 255), QColor(20, 80, 160));
    rect->setData(DATA_TYPE, PLAYER_SPAWN_TYPE);
    rect->setData(DATA_ID, id);
    attachLabel(rect, QStringLiteral("Spawn"));
    return rect;
}

QGraphicsRectItem* ItemBuilder::buildObstacle(const QString& id, const QString& type, int width,
                                              int height, const QColor& fill) {
    auto* rect = new QGraphicsRectItem(0, 0, width * CELL_DISPLAY_SIZE, height * CELL_DISPLAY_SIZE);
    rect->setBrush(QBrush(fill));
    rect->setPen(QPen(fill.darker(160), 1));
    rect->setData(DATA_TYPE, OBSTACLE_TYPE);
    rect->setData(DATA_ID, id);
    rect->setData(DATA_SUBTYPE, type);
    rect->setData(DATA_WIDTH, width);
    rect->setData(DATA_HEIGHT, height);
    attachLabel(rect, type);
    return rect;
}

QGraphicsRectItem* ItemBuilder::buildEntry(const QString& id, const QString& type,
                                           const QString& environmentId, int width, int height,
                                           const QColor& fill) {
    auto* rect = new QGraphicsRectItem(0, 0, width * CELL_DISPLAY_SIZE, height * CELL_DISPLAY_SIZE);
    rect->setBrush(QBrush(fill));
    rect->setPen(QPen(fill.darker(180), 2));
    rect->setData(DATA_TYPE, ENTRY_TYPE);
    rect->setData(DATA_ID, id);
    rect->setData(DATA_SUBTYPE, type);
    rect->setData(DATA_WIDTH, width);
    rect->setData(DATA_HEIGHT, height);
    rect->setData(DATA_ENVIRONMENT_ID, environmentId);
    attachLabel(rect, type);
    return rect;
}

QGraphicsRectItem* ItemBuilder::buildWall(const QString& id, const QString& templateId, int width,
                                          int height, const QColor& fill) {
    auto* rect = new QGraphicsRectItem(0, 0, width * CELL_DISPLAY_SIZE, height * CELL_DISPLAY_SIZE);
    rect->setBrush(QBrush(fill));
    rect->setPen(QPen(fill.darker(180), 1));
    rect->setData(DATA_TYPE, WALL_TYPE);
    rect->setData(DATA_ID, id);
    rect->setData(DATA_SUBTYPE, templateId);
    rect->setData(DATA_WIDTH, width);
    rect->setData(DATA_HEIGHT, height);
    return rect;
}

QGraphicsRectItem* ItemBuilder::buildZone(const QString& id, const QString& zoneType,
                                          const QString& templateId, int width, int height,
                                          const QColor& fill) {
    const bool isCity = zoneType == ZONE_TYPE_CITY;
    auto* rect = new QGraphicsRectItem(0, 0, width * CELL_DISPLAY_SIZE, height * CELL_DISPLAY_SIZE);

    QColor fillTransparent(fill);
    fillTransparent.setAlpha(120);
    QColor border = fill.darker(160);
    border.setAlpha(255);

    rect->setBrush(QBrush(fillTransparent));
    rect->setPen(QPen(border, 2));
    rect->setData(DATA_TYPE, isCity ? CITY_ZONE_TYPE : BIOME_ZONE_TYPE);
    rect->setData(DATA_ID, id);
    rect->setData(DATA_SUBTYPE, templateId);
    rect->setData(DATA_WIDTH, width);
    rect->setData(DATA_HEIGHT, height);
    attachLabel(rect, QStringLiteral("%1 (%2)").arg(templateId, zoneType));
    return rect;
}
