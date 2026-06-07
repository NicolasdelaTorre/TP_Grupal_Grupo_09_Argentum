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
                                     const QColor& fill, const QString& texturePath = QString());
    QGraphicsRectItem* buildZone(const QString& id, const QString& zoneType,
                                 const QString& templateId, int width, int height,
                                 const QColor& fill);
    QGraphicsRectItem* buildEntry(const QString& id, const QString& type,
                                  const QString& environmentId, int width, int height,
                                  const QColor& fill);
    QGraphicsRectItem* buildWall(const QString& id, const QString& templateId, int width,
                                 int height, const QColor& fill);
    QGraphicsRectItem* buildFloor(const QString& id, const QString& templateId, const QColor& fill,
                                  const QString& texturePath = QString());

private:
    static QGraphicsRectItem* make_rect(const QColor& fill, const QColor& border);
    void attachLabel(QGraphicsRectItem* rect, const QString& text);
};

#endif
