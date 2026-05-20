#ifndef ARGENTUM_EDITOR_MAP_CANVAS_H
#define ARGENTUM_EDITOR_MAP_CANVAS_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>

#include "map/map_data.h"
#include "assets/templates/template_registry.h"
#include "editor_constants.h"
#include "scene_controller.h"
#include "tool_info.h"

class MapCanvas: public QWidget {
    Q_OBJECT

public:
    explicit MapCanvas(const TemplateRegistry& templates, QWidget* parent = nullptr);

    void createMap(const QString& map_id, const QString& map_name, int width, int height);
    void setActiveTool(const ToolInfo& tool);

    QString map_id() const;
    QString map_name() const;
    int map_width() const;
    int map_height() const;

signals:
    void statusMessage(const QString& message);

public slots:
    bool saveMap();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    const TemplateRegistry& templates_;
    QGraphicsScene* scene_;
    QGraphicsView* view_;
    SceneController* controller_;

    QString map_id_;
    QString map_name_;
    int map_width_ = 0;
    int map_height_ = 0;
    ToolInfo active_tool_;
    double current_zoom_ = INITIAL_ZOOM;

    bool drawing_zone_ = false;
    QPoint zone_start_cell_;
    QGraphicsRectItem* zone_preview_ = nullptr;

    void drawGrid();
    void applyInitialView();
    void cellFromViewPos(const QPoint& view_pos, int& cell_x, int& cell_y) const;
    void handleLeftPress(const QPoint& view_pos);
    void handleRightPress(const QPoint& view_pos);
    void handleMouseMove(const QPoint& view_pos);
    void placeCityAt(int cell_x, int cell_y);
    void finishForestZoneDraw(int end_cell_x, int end_cell_y);
    void clearZonePreview();
    QRect normalizedCellRect(const QPoint& a, const QPoint& b) const;
    void zoomIn();
    void zoomOut();
};

#endif
