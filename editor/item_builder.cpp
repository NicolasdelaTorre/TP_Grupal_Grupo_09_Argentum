#include "item_builder.h"

#include <QGraphicsPixmapItem>
#include <QPen>
#include <QPixmap>

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
                                              int height, const QColor& fill,
                                              const QString& texturePath) {
    const int pixel_w = width * CELL_DISPLAY_SIZE;
    const int pixel_h = height * CELL_DISPLAY_SIZE;
    auto* rect = new QGraphicsRectItem(0, 0, pixel_w, pixel_h);

    // dibuja a tamaño nativo, ancla esquina inferior izquierda a esquina inferior izquierda del rect
    QPixmap pixmap;
    const bool has_texture = !texturePath.isEmpty() && pixmap.load(texturePath);
    if (has_texture) {
        rect->setBrush(Qt::NoBrush);
        rect->setPen(Qt::NoPen);
        auto* texture_item = new QGraphicsPixmapItem(pixmap, rect);
        texture_item->setTransformationMode(Qt::SmoothTransformation);
        texture_item->setPos(0, pixel_h - pixmap.height());
    } else {
        rect->setBrush(QBrush(fill));
        rect->setPen(QPen(fill.darker(160), 1));
    }

    rect->setData(DATA_TYPE, OBSTACLE_TYPE);
    rect->setData(DATA_ID, id);
    rect->setData(DATA_SUBTYPE, type);
    rect->setData(DATA_WIDTH, width);
    rect->setData(DATA_HEIGHT, height);
    if (!has_texture) {
        attachLabel(rect, type);
    }
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

QGraphicsRectItem* ItemBuilder::buildFloor(const QString& id, const QString& templateId,
                                           const QColor& fill, const QString& texturePath) {
    auto* rect = new QGraphicsRectItem(0, 0, CELL_DISPLAY_SIZE, CELL_DISPLAY_SIZE);

    // Las texturas de piso ya vienen a 64x64 (= CELL_DISPLAY_SIZE), se dibujan
    // tal cual cubriendo la celda. Si no hay textura, se rellena con el color.
    QPixmap pixmap;
    const bool has_texture = !texturePath.isEmpty() && pixmap.load(texturePath);
    if (has_texture) {
        rect->setBrush(Qt::NoBrush);
        rect->setPen(Qt::NoPen);
        auto* texture_item = new QGraphicsPixmapItem(pixmap, rect);
        texture_item->setTransformationMode(Qt::SmoothTransformation);
        texture_item->setPos(0, 0);
    } else {
        rect->setBrush(QBrush(fill));
        rect->setPen(Qt::NoPen);
    }

    rect->setData(DATA_TYPE, FLOOR_TYPE);
    rect->setData(DATA_ID, id);
    rect->setData(DATA_SUBTYPE, templateId);
    rect->setData(DATA_WIDTH, 1);
    rect->setData(DATA_HEIGHT, 1);
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
