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
    QGraphicsRectItem* buildObstacle(const QString& id, const QString& type, int width, int height);
    QGraphicsRectItem* buildZone(const QString& id, const QString& zoneType,
                                 const QString& templateId, int width, int height);

private:
    static QGraphicsRectItem* make_rect(const QColor& fill, const QColor& border);
    void attachLabel(QGraphicsRectItem* rect, const QString& text);
};

#endif
