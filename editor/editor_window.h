#ifndef ARGENTUM_EDITOR_EDITOR_WINDOW_H
#define ARGENTUM_EDITOR_EDITOR_WINDOW_H

#include <QButtonGroup>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QString>

#include "assets/templates/template_registry.h"
#include "map/map_data.h"

#include "map_canvas.h"
#include "tool_info.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class EditorWindow;
}
QT_END_NAMESPACE

class EditorWindow: public QMainWindow {
    Q_OBJECT

public:
    explicit EditorWindow(QWidget* parent = nullptr);
    ~EditorWindow() override;

private:
    enum class ResizeDirection { Left, Right, Up, Down };

    struct ResizeDelta {
        int delta_w = 0;
        int delta_h = 0;
        int offset_x = 0;
        int offset_y = 0;
    };

    Ui::EditorWindow* ui_;
    TemplateRegistry templates_;
    MapCanvas* map_canvas_ = nullptr;
    QButtonGroup* tool_group_;
    ToolInfo active_tool_;

    MapDocument main_doc_;
    QString current_environment_id_;
    int next_entry_index_ = 1;
    int next_environment_index_ = 1;

    static ResizeDelta computeResizeDelta(ResizeDirection dir, int cells, bool shrink);
    static bool fitsInside(int x, int y, int w, int h, int map_w, int map_h);
    static bool canShrinkDocument(const MapDocument& doc, const ResizeDelta& delta);
    static void applyResizeToDocument(MapDocument& doc, const ResizeDelta& delta);

    void setupTemplates();
    void setupTools();
    void setupNewMapPage();
    void resetNewMapPage();
    void onCreateNewMap();
    void openExistingMap();
    void applyActiveTool();
    void selectTool(EditorTool tool);
    void selectSpawnMode();
    void selectObstacleMode();
    void selectBiomeMode();
    void selectCityMode();
    void selectEnvironmentMode();
    void selectDimensionsMode();
    void selectDefaultMode();
    void updateDimensionsLabel();
    void onApplyMapResize();

    void startNewMainMap(const QString& map_id, const QString& map_name, int width, int height);
    void onEntryPlacementRequested(const QString& template_id, int cell_x, int cell_y);
    void onEntryDeleted(const QString& environment_id);
    void onEnvironmentDoubleClicked(QListWidgetItem* item);
    void backToMainMap();
    void saveCurrentToDocument();
    void enterEnvironment(const QString& environment_id);
    void refreshEnvironmentsList();
    void saveMap();
    void setMainOnlySectionsVisible(bool visible);

    Environment* find_environment(const QString& id);
    const Environment* find_environment(const QString& id) const;
};

#endif
