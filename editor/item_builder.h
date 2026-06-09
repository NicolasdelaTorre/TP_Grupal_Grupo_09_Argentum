#ifndef ARGENTUM_EDITOR_ITEM_BUILDER_H
#define ARGENTUM_EDITOR_ITEM_BUILDER_H

#include <QBrush>
#include <QColor>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QString>

class ItemBuilder {
public:
    QGraphicsRectItem* buildPlayerSpawn(const QString& id);
    QGraphicsRectItem* buildObstacle(const QString& id, const QString& type, int width, int height,
                                     const QString& texturePath);
    QGraphicsRectItem* buildZone(const QString& id, const QString& zoneType,
                                 const QString& templateId, int width, int height,
                                 const QColor& fill);
    QGraphicsRectItem* buildEntry(const QString& id, const QString& type,
                                  const QString& environmentId, int width, int height,
                                  const QString& texturePath);
    QGraphicsRectItem* buildWall(const QString& id, const QString& templateId, int width,
                                 int height, const QString& texturePath);
    QGraphicsRectItem* buildExit(const QString& id, const QString& templateId, int width,
                                 int height, const QString& texturePath);
    QGraphicsRectItem* buildFloor(const QString& id, const QString& templateId,
                                  const QString& texturePath);

private:
    void attachLabel(QGraphicsRectItem* rect, const QString& text);
};

#endif
