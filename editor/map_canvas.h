#ifndef ARGENTUM_EDITOR_MAP_CANVAS_H
#define ARGENTUM_EDITOR_MAP_CANVAS_H

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>

#include <vector>

#include "assets/templates/template_registry.h"
#include "map/map_data.h"

#include "editor_constants.h"
#include "scene_controller.h"
#include "tool_info.h"

enum class EditingMode {
    MainMap,
    Environment,
};

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
    EditingMode editing_mode() const;

    MapDocument buildDocument() const;
    void loadFromDocument(const MapDocument& document, EditingMode mode);

    bool placeEntryItem(const QString& entry_id, const QString& environment_id,
                        const QString& template_id, int cell_x, int cell_y);

signals:
    void statusMessage(const QString& message);
    void entryPlacementRequested(const QString& template_id, int cell_x, int cell_y);
    void entryDeleted(const QString& environment_id);
    void saveRequested();
    void biomeHoverInfo(const QString& text);

public slots:
    void onSaveClicked();

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
    EditingMode editing_mode_ = EditingMode::MainMap;

    bool drawing_zone_ = false;
    QPoint zone_start_cell_;
    QGraphicsRectItem* zone_preview_ = nullptr;
    QGraphicsPixmapItem* biome_tint_item_ = nullptr;
    // Items renderizados por celda para los biomas que tienen textura. Se reconstruyen
    // en cada rebuildBiomeTint().
    std::vector<QGraphicsPixmapItem*> biome_texture_items_;
    QGraphicsRectItem* env_floor_item_ = nullptr;
    QGraphicsPixmapItem* env_exterior_item_ = nullptr;
    QString env_floor_color_;
    QString last_hover_zone_id_;

    void initializeScene(const QString& map_id, const QString& map_name, int width, int height);
    void drawGrid();
    void applyInitialView();
    void cellFromViewPos(const QPoint& view_pos, int& cell_x, int& cell_y) const;
    void handleLeftPress(const QPoint& view_pos);
    void handleRightPress(const QPoint& view_pos);
    void handleMouseMove(const QPoint& view_pos);
    void handleHoverMove(const QPoint& view_pos);
    void placeCityAt(int cell_x, int cell_y);
    void placeObstacleAt(int cell_x, int cell_y);
    void placeWallAt(int cell_x, int cell_y);
    void requestEntryAt(int cell_x, int cell_y);
    void finishBiomeZoneDraw(int end_cell_x, int end_cell_y);
    QGraphicsItem* biomeZoneAtCell(int cell_x, int cell_y) const;
    void editBiomeSpawnsAt(int cell_x, int cell_y);
    void clearZonePreview();
    QRect normalizedCellRect(const QPoint& a, const QPoint& b) const;
    void zoomIn();
    void zoomOut();
    void rebuildBiomeTint();
    void rebuildEnvironmentLayers();
    bool isToolAllowed(EditorTool tool) const;
};

#endif
