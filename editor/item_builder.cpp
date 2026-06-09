#include "item_builder.h"

#include <QGraphicsPixmapItem>
#include <QPen>
#include <QPixmap>

#include "editor_constants.h"

void ItemBuilder::attachLabel(QGraphicsRectItem* rect, const QString& text) {
    auto* label = new QGraphicsSimpleTextItem(text, rect);
    label->setPos(2, 2);
}

QGraphicsRectItem* ItemBuilder::buildPlayerSpawn(const QString& id) {
    auto* rect = new QGraphicsRectItem(0, 0, CELL_DISPLAY_SIZE, CELL_DISPLAY_SIZE);
    rect->setBrush(Qt::NoBrush);
    rect->setPen(Qt::NoPen);

    // El spawn se muestra con una textura propia (placeholder en ui_textures).
    // Mientras esa textura no exista, se deja un marcador mínimo con label para
    // que el punto siga siendo visible en el editor.
    QPixmap pixmap;
    if (pixmap.load(QStringLiteral(PLAYER_SPAWN_TEXTURE))) {
        auto* texture_item = new QGraphicsPixmapItem(
                pixmap.scaled(CELL_DISPLAY_SIZE, CELL_DISPLAY_SIZE, Qt::IgnoreAspectRatio,
                              Qt::SmoothTransformation),
                rect);
        texture_item->setTransformationMode(Qt::SmoothTransformation);
        texture_item->setPos(0, 0);
    } else {
        rect->setPen(QPen(QColor(20, 80, 160), 1));
        attachLabel(rect, QStringLiteral("Spawn"));
    }

    rect->setData(DATA_TYPE, PLAYER_SPAWN_TYPE);
    rect->setData(DATA_ID, id);
    return rect;
}

QGraphicsRectItem* ItemBuilder::buildObstacle(const QString& id, const QString& type, int width,
                                              int height, const QString& texturePath) {
    const int pixel_w = width * CELL_DISPLAY_SIZE;
    const int pixel_h = height * CELL_DISPLAY_SIZE;
    auto* rect = new QGraphicsRectItem(0, 0, pixel_w, pixel_h);
    rect->setBrush(Qt::NoBrush);
    rect->setPen(Qt::NoPen);

    // dibuja a tamaño nativo, ancla esquina inferior izquierda a esquina inferior izquierda del rect
    QPixmap pixmap;
    if (!texturePath.isEmpty() && pixmap.load(texturePath)) {
        auto* texture_item = new QGraphicsPixmapItem(pixmap, rect);
        texture_item->setTransformationMode(Qt::SmoothTransformation);
        texture_item->setPos(0, pixel_h - pixmap.height());
    }

    rect->setData(DATA_TYPE, OBSTACLE_TYPE);
    rect->setData(DATA_ID, id);
    rect->setData(DATA_SUBTYPE, type);
    rect->setData(DATA_WIDTH, width);
    rect->setData(DATA_HEIGHT, height);
    return rect;
}

QGraphicsRectItem* ItemBuilder::buildEntry(const QString& id, const QString& type,
                                           const QString& environmentId, int width, int height,
                                           const QString& texturePath) {
    const int pixel_w = width * CELL_DISPLAY_SIZE;
    const int pixel_h = height * CELL_DISPLAY_SIZE;
    auto* rect = new QGraphicsRectItem(0, 0, pixel_w, pixel_h);
    rect->setBrush(Qt::NoBrush);
    rect->setPen(Qt::NoPen);

    // dibuja a tamaño nativo, ancla esquina inferior izquierda a esquina inferior izquierda del rect
    QPixmap pixmap;
    if (!texturePath.isEmpty() && pixmap.load(texturePath)) {
        auto* texture_item = new QGraphicsPixmapItem(pixmap, rect);
        texture_item->setTransformationMode(Qt::SmoothTransformation);
        texture_item->setPos(0, pixel_h - pixmap.height());
    }

    rect->setData(DATA_TYPE, ENTRY_TYPE);
    rect->setData(DATA_ID, id);
    rect->setData(DATA_SUBTYPE, type);
    rect->setData(DATA_WIDTH, width);
    rect->setData(DATA_HEIGHT, height);
    rect->setData(DATA_ENVIRONMENT_ID, environmentId);
    return rect;
}

QGraphicsRectItem* ItemBuilder::buildWall(const QString& id, const QString& templateId, int width,
                                          int height, const QString& texturePath) {
    const int pixel_w = width * CELL_DISPLAY_SIZE;
    const int pixel_h = height * CELL_DISPLAY_SIZE;
    auto* rect = new QGraphicsRectItem(0, 0, pixel_w, pixel_h);

    // dibuja a tamaño nativo, ancla esquina inferior izquierda a esquina inferior izquierda del rect
    QPixmap pixmap;
    const bool has_texture = !texturePath.isEmpty() && pixmap.load(texturePath);
    rect->setBrush(Qt::NoBrush);
    rect->setPen(Qt::NoPen);
    if (has_texture) {
        auto* texture_item = new QGraphicsPixmapItem(pixmap, rect);
        texture_item->setTransformationMode(Qt::SmoothTransformation);
        texture_item->setPos(0, pixel_h - pixmap.height());
    }

    rect->setData(DATA_TYPE, WALL_TYPE);
    rect->setData(DATA_ID, id);
    rect->setData(DATA_SUBTYPE, templateId);
    rect->setData(DATA_WIDTH, width);
    rect->setData(DATA_HEIGHT, height);
    return rect;
}

QGraphicsRectItem* ItemBuilder::buildExit(const QString& id, const QString& templateId, int width,
                                          int height, const QString& texturePath) {
    const int pixel_w = width * CELL_DISPLAY_SIZE;
    const int pixel_h = height * CELL_DISPLAY_SIZE;
    auto* rect = new QGraphicsRectItem(0, 0, pixel_w, pixel_h);

    QPixmap pixmap;
    const bool has_texture = !texturePath.isEmpty() && pixmap.load(texturePath);
    rect->setBrush(Qt::NoBrush);
    rect->setPen(Qt::NoPen);
    if (has_texture) {
        auto* texture_item = new QGraphicsPixmapItem(pixmap, rect);
        texture_item->setTransformationMode(Qt::SmoothTransformation);
        texture_item->setPos(0, pixel_h - pixmap.height());
    }

    rect->setData(DATA_TYPE, EXIT_TYPE);
    rect->setData(DATA_ID, id);
    rect->setData(DATA_SUBTYPE, templateId);
    rect->setData(DATA_WIDTH, width);
    rect->setData(DATA_HEIGHT, height);
    return rect;
}

QGraphicsRectItem* ItemBuilder::buildFloor(const QString& id, const QString& templateId,
                                           const QString& texturePath) {
    auto* rect = new QGraphicsRectItem(0, 0, CELL_DISPLAY_SIZE, CELL_DISPLAY_SIZE);
    rect->setBrush(Qt::NoBrush);
    rect->setPen(Qt::NoPen);

    // Las texturas de piso ya vienen a 64x64 (= CELL_DISPLAY_SIZE), se dibujan
    // tal cual cubriendo la celda.
    QPixmap pixmap;
    if (!texturePath.isEmpty() && pixmap.load(texturePath)) {
        auto* texture_item = new QGraphicsPixmapItem(pixmap, rect);
        texture_item->setTransformationMode(Qt::SmoothTransformation);
        texture_item->setPos(0, 0);
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
