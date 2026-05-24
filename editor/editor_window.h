#ifndef ARGENTUM_EDITOR_EDITOR_WINDOW_H
#define ARGENTUM_EDITOR_EDITOR_WINDOW_H

#include <QButtonGroup>
#include <QMainWindow>

#include "assets/templates/template_registry.h"

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
    Ui::EditorWindow* ui_;
    TemplateRegistry templates_;
    MapCanvas* map_canvas_ = nullptr;
    QButtonGroup* tool_group_;
    ToolInfo active_tool_;

    void setupTemplates();
    void setupTools();
    void applyActiveTool();
    void selectTool(EditorTool tool);
};

#endif
