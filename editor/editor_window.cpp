#include "editor_window.h"

#include <QMessageBox>

#include "dialogs/new_map_dialog.h"
#include "editor_constants.h"
#include "ui_EditorWindow.h"
// ventana del editor
EditorWindow::EditorWindow(QWidget* parent):
        QMainWindow(parent),
        ui_(new Ui::EditorWindow),
        tool_group_(new QButtonGroup(this)) {
    ui_->setupUi(this);

    if (!templates_.load()) {
        QMessageBox::critical(this, QStringLiteral("Error"),
                             QStringLiteral("No se pudieron cargar los templates en %1.")
                                     .arg(QStringLiteral(TEMPLATES_PATH)));
    }

    map_canvas_ = new MapCanvas(templates_, ui_->mapCanvasHost);
    ui_->mapCanvasHost->layout()->addWidget(map_canvas_);

    setupTemplates();
    setupTools();

    connect(ui_->btnNewMap, &QPushButton::clicked, this, [this]() {
        NewMapDialog dialog(this);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }
        if (dialog.map_id().isEmpty()) {
            QMessageBox::warning(this, QStringLiteral("Mapa inválido"),
                                 QStringLiteral("El id del mapa no puede estar vacío."));
            return;
        }

        map_canvas_->createMap(dialog.map_id(), dialog.map_name(), dialog.map_width(),
                               dialog.map_height());
        ui_->stackedWidget->setCurrentWidget(ui_->pageEditor);
    });

    connect(ui_->btnBack, &QPushButton::clicked, this,
            [this] { ui_->stackedWidget->setCurrentWidget(ui_->pageMainMenu); });

    connect(map_canvas_, &MapCanvas::statusMessage, ui_->labelStatus, &QLabel::setText);
}

EditorWindow::~EditorWindow() { delete ui_; }

void EditorWindow::setupTemplates() {
    for (const auto& city: templates_.cities()) {
        ui_->comboCityTemplate->addItem(QString::fromStdString(city.name),
                                        QString::fromStdString(city.id));
    }
    for (const auto& forest: templates_.forests()) {
        ui_->comboForestTemplate->addItem(QString::fromStdString(forest.name),
                                         QString::fromStdString(forest.id));
    }
}
// setup herramientas
void EditorWindow::setupTools() {
    // botones
    tool_group_->setExclusive(true);
    tool_group_->addButton(ui_->btnToolSpawn);
    tool_group_->addButton(ui_->btnToolObstacle);
    tool_group_->addButton(ui_->btnToolCity);
    tool_group_->addButton(ui_->btnToolForest);

    ui_->comboObstacleType->addItems(
            {QStringLiteral("tree"), QStringLiteral("piedra_grande")});
    // conectar los botones a la herramienta seleccionada
    connect(ui_->btnToolSpawn, &QPushButton::clicked, this,
            [this]() { selectTool(EditorTool::PlayerSpawn); });
    connect(ui_->btnToolObstacle, &QPushButton::clicked, this,
            [this]() { selectTool(EditorTool::Obstacle); });
    connect(ui_->btnToolCity, &QPushButton::clicked, this,
            [this]() { selectTool(EditorTool::CityZone); });
    connect(ui_->btnToolForest, &QPushButton::clicked, this,
            [this]() { selectTool(EditorTool::ForestZone); });
    // si se actualizan las selecciones, actualizar la herramienta actual
    connect(ui_->comboObstacleType, &QComboBox::currentTextChanged, this,
            [this](const QString&) { applyActiveTool(); });
    connect(ui_->spinObstacleWidth, &QSpinBox::valueChanged, this,
            [this](int) { applyActiveTool(); });
    connect(ui_->spinObstacleHeight, &QSpinBox::valueChanged, this,
            [this](int) { applyActiveTool(); });
    connect(ui_->comboCityTemplate, &QComboBox::currentIndexChanged, this,
            [this](int) { applyActiveTool(); });
    connect(ui_->comboForestTemplate, &QComboBox::currentIndexChanged, this,
            [this](int) { applyActiveTool(); });
}
// seleccionar la herramienta actual
void EditorWindow::selectTool(EditorTool tool) {
    active_tool_.tool = tool;
    applyActiveTool();
}
// actualizar herramienta actual
void EditorWindow::applyActiveTool() {
    active_tool_.obstacle_type = ui_->comboObstacleType->currentText();
    active_tool_.obstacle_width = ui_->spinObstacleWidth->value();
    active_tool_.obstacle_height = ui_->spinObstacleHeight->value();
    active_tool_.city_template_id = ui_->comboCityTemplate->currentData().toString();
    active_tool_.forest_template_id = ui_->comboForestTemplate->currentData().toString();
    map_canvas_->setActiveTool(active_tool_);
}
