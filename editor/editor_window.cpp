#include "editor_window.h"

#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QSize>

#include "dialogs/new_environment_dialog.h"
#include "editor_constants.h"
#include "map/yaml_map_io.h"
#include "ui_EditorWindow.h"
#include "verificator.h"

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
    setupNewMapPage();

    connect(ui_->btnNewMap, &QPushButton::clicked, this, [this]() {
        resetNewMapPage();
        ui_->stackedWidget->setCurrentWidget(ui_->pageNewMap);
    });

    connect(ui_->btnNewMapCancel, &QPushButton::clicked, this,
            [this] { ui_->stackedWidget->setCurrentWidget(ui_->pageMainMenu); });

    connect(ui_->btnNewMapCreate, &QPushButton::clicked, this, &EditorWindow::onCreateNewMap);

    connect(ui_->btnBack, &QPushButton::clicked, this,
            [this] { ui_->stackedWidget->setCurrentWidget(ui_->pageMainMenu); });

    connect(ui_->btnBackToMainMap, &QPushButton::clicked, this,
            [this]() { backToMainMap(); });

    connect(ui_->listEnvironments, &QListWidget::itemDoubleClicked, this,
            &EditorWindow::onEnvironmentDoubleClicked);

    connect(map_canvas_, &MapCanvas::statusMessage, ui_->labelStatus, &QLabel::setText);
    connect(map_canvas_, &MapCanvas::entryPlacementRequested, this,
            &EditorWindow::onEntryPlacementRequested);
    connect(map_canvas_, &MapCanvas::entryDeleted, this, &EditorWindow::onEntryDeleted);
    connect(map_canvas_, &MapCanvas::saveRequested, this, &EditorWindow::saveMap);
}

EditorWindow::~EditorWindow() { delete ui_; }

void EditorWindow::setupTemplates() {
    for (const auto& city: templates_.cities()) {
        ui_->comboCityTemplate->addItem(QString::fromStdString(city.name),
                                        QString::fromStdString(city.id));
    }
    for (const auto& biome: templates_.biomes()) {
        ui_->comboBiomeTemplate->addItem(QString::fromStdString(biome.name),
                                         QString::fromStdString(biome.id));
    }
    for (const auto& obstacle: templates_.obstacles()) {
        const QString label = QStringLiteral("%1 (%2x%3)")
                                      .arg(QString::fromStdString(obstacle.name))
                                      .arg(obstacle.width)
                                      .arg(obstacle.height);
        ui_->comboObstacleTemplate->addItem(label, QString::fromStdString(obstacle.id));
    }
    for (const auto& entry: templates_.entries()) {
        const QString label = QStringLiteral("%1 (%2x%3)")
                                      .arg(QString::fromStdString(entry.name))
                                      .arg(entry.width)
                                      .arg(entry.height);
        ui_->comboEntryTemplate->addItem(label, QString::fromStdString(entry.id));
    }
    for (const auto& wall: templates_.walls()) {
        const QString label = QStringLiteral("%1 (%2x%3)")
                                      .arg(QString::fromStdString(wall.name))
                                      .arg(wall.width)
                                      .arg(wall.height);
        ui_->comboWallTemplate->addItem(label, QString::fromStdString(wall.id));
    }
}

void EditorWindow::setupTools() {
    tool_group_->setExclusive(true);
    tool_group_->addButton(ui_->btnToolSpawn);
    tool_group_->addButton(ui_->btnToolObstacle);
    tool_group_->addButton(ui_->btnToolCity);
    tool_group_->addButton(ui_->btnToolBiome);
    tool_group_->addButton(ui_->btnToolEntry);
    tool_group_->addButton(ui_->btnToolWall);

    connect(ui_->btnToolSpawn, &QPushButton::clicked, this,
            [this]() { selectTool(EditorTool::PlayerSpawn); });
    connect(ui_->btnToolObstacle, &QPushButton::clicked, this,
            [this]() { selectTool(EditorTool::Obstacle); });
    connect(ui_->btnToolCity, &QPushButton::clicked, this,
            [this]() { selectTool(EditorTool::CityZone); });
    connect(ui_->btnToolBiome, &QPushButton::clicked, this,
            [this]() { selectTool(EditorTool::BiomeZone); });
    connect(ui_->btnToolEntry, &QPushButton::clicked, this,
            [this]() { selectTool(EditorTool::Entry); });
    connect(ui_->btnToolWall, &QPushButton::clicked, this,
            [this]() { selectTool(EditorTool::Wall); });

    connect(ui_->comboObstacleTemplate, &QComboBox::currentIndexChanged, this,
            [this](int) { applyActiveTool(); });
    connect(ui_->comboCityTemplate, &QComboBox::currentIndexChanged, this,
            [this](int) { applyActiveTool(); });
    connect(ui_->comboBiomeTemplate, &QComboBox::currentIndexChanged, this,
            [this](int) { applyActiveTool(); });
    connect(ui_->comboEntryTemplate, &QComboBox::currentIndexChanged, this,
            [this](int) { applyActiveTool(); });
    connect(ui_->comboWallTemplate, &QComboBox::currentIndexChanged, this,
            [this](int) { applyActiveTool(); });
}

void EditorWindow::selectTool(EditorTool tool) {
    active_tool_.tool = tool;
    applyActiveTool();
}

void EditorWindow::applyActiveTool() {
    active_tool_.obstacle_template_id = ui_->comboObstacleTemplate->currentData().toString();
    active_tool_.city_template_id = ui_->comboCityTemplate->currentData().toString();
    active_tool_.biome_template_id = ui_->comboBiomeTemplate->currentData().toString();
    active_tool_.entry_template_id = ui_->comboEntryTemplate->currentData().toString();
    active_tool_.wall_template_id = ui_->comboWallTemplate->currentData().toString();
    map_canvas_->setActiveTool(active_tool_);
}

void EditorWindow::setupNewMapPage() {
    ui_->comboNewMapSize->clear();
    ui_->comboNewMapSize->addItem(QStringLiteral("100 x 100"), QSize(100, 100));
    ui_->comboNewMapSize->addItem(QStringLiteral("250 x 250"), QSize(250, 250));
    ui_->comboNewMapSize->addItem(QStringLiteral("500 x 500"), QSize(500, 500));
}

void EditorWindow::resetNewMapPage() {
    ui_->inputNewMapId->setText(QStringLiteral("mapa_inicial"));
    ui_->inputNewMapName->setText(QStringLiteral("Mapa Inicial"));
    ui_->comboNewMapSize->setCurrentIndex(0);
}

void EditorWindow::onCreateNewMap() {
    const QString map_id = ui_->inputNewMapId->text().trimmed();
    const QString map_name = ui_->inputNewMapName->text().trimmed();
    if (map_id.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Mapa inválido"),
                             QStringLiteral("El id del mapa no puede estar vacío."));
        return;
    }

    const QSize size = ui_->comboNewMapSize->currentData().toSize();
    startNewMainMap(map_id, map_name, size.width(), size.height());
    ui_->stackedWidget->setCurrentWidget(ui_->pageEditor);
}

void EditorWindow::startNewMainMap(const QString& map_id, const QString& map_name, int width,
                                   int height) {
    main_doc_ = MapDocument();
    main_doc_.version = 1;
    main_doc_.map.id = map_id.toStdString();
    main_doc_.map.name = map_name.toStdString();
    main_doc_.map.width = width;
    main_doc_.map.height = height;
    current_environment_id_.clear();
    next_entry_index_ = 1;
    next_environment_index_ = 1;

    map_canvas_->createMap(map_id, map_name, width, height);
    refreshEnvironmentsList();
    ui_->labelEditingTarget->setText(QStringLiteral("Editando: mapa principal"));
    ui_->btnBackToMainMap->setVisible(false);
    setMainOnlySectionsVisible(true);
}

void EditorWindow::onEntryPlacementRequested(const QString& template_id, int cell_x, int cell_y) {
    const auto* entry_template = templates_.find_entry(template_id.toStdString());
    if (!entry_template) {
        return;
    }
    if (entry_template->environment_sizes.empty()) {
        QMessageBox::warning(this, QStringLiteral("Entrada"),
                             QStringLiteral("El template no define tamaños de entorno."));
        return;
    }

    const QString suggested = QStringLiteral("%1 %2")
                                      .arg(QString::fromStdString(entry_template->name))
                                      .arg(next_environment_index_);
    NewEnvironmentDialog dialog(*entry_template, suggested, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString env_id = QStringLiteral("env_%1").arg(next_environment_index_++);
    QString entry_id = QStringLiteral("entry_%1").arg(next_entry_index_++);

    Environment env;
    env.id = env_id.toStdString();
    env.name = dialog.environment_name().toStdString();
    env.type = entry_template->id;
    env.width = dialog.environment_width();
    env.height = dialog.environment_height();
    env.floor_color = entry_template->floor_color;
    main_doc_.environments.push_back(env);

    if (!map_canvas_->placeEntryItem(entry_id, env_id, template_id, cell_x, cell_y)) {
        main_doc_.environments.pop_back();
        next_environment_index_--;
        next_entry_index_--;
        return;
    }

    refreshEnvironmentsList();
}

void EditorWindow::onEntryDeleted(const QString& environment_id) {
    auto& envs = main_doc_.environments;
    envs.erase(std::remove_if(envs.begin(), envs.end(),
                              [&](const Environment& env) {
                                  return QString::fromStdString(env.id) == environment_id;
                              }),
               envs.end());
    refreshEnvironmentsList();
}

void EditorWindow::onEnvironmentDoubleClicked(QListWidgetItem* item) {
    if (!item) {
        return;
    }
    const QString env_id = item->data(Qt::UserRole).toString();
    if (env_id.isEmpty()) {
        return;
    }
    enterEnvironment(env_id);
}

void EditorWindow::backToMainMap() {
    if (map_canvas_->editing_mode() == EditingMode::MainMap) {
        return;
    }
    saveCurrentToDocument();
    current_environment_id_.clear();
    map_canvas_->loadFromDocument(main_doc_, EditingMode::MainMap);
    refreshEnvironmentsList();
    ui_->labelEditingTarget->setText(QStringLiteral("Editando: mapa principal"));
    ui_->btnBackToMainMap->setVisible(false);
    setMainOnlySectionsVisible(true);
    selectTool(EditorTool::None);
}

void EditorWindow::saveCurrentToDocument() {
    const MapDocument current = map_canvas_->buildDocument();
    if (map_canvas_->editing_mode() == EditingMode::MainMap) {
        const auto saved_envs = main_doc_.environments;
        main_doc_ = current;
        main_doc_.environments = saved_envs;
        return;
    }

    Environment* env = find_environment(current_environment_id_);
    if (!env) {
        return;
    }
    env->obstacles = current.obstacles;
    env->player_spawn = current.player_spawn;
    env->walls = current.walls;
}

void EditorWindow::enterEnvironment(const QString& environment_id) {
    saveCurrentToDocument();
    const Environment* env = find_environment(environment_id);
    if (!env) {
        return;
    }
    current_environment_id_ = environment_id;

    MapDocument env_doc;
    env_doc.version = 1;
    env_doc.map.id = env->id;
    env_doc.map.name = env->name;
    env_doc.map.width = env->width;
    env_doc.map.height = env->height;
    env_doc.player_spawn = env->player_spawn;
    env_doc.obstacles = env->obstacles;
    env_doc.walls = env->walls;
    env_doc.floor_color = env->floor_color;
    if (env_doc.floor_color.empty()) {
        if (const auto* entry_tpl = templates_.find_entry(env->type)) {
            env_doc.floor_color = entry_tpl->floor_color;
        }
    }

    map_canvas_->loadFromDocument(env_doc, EditingMode::Environment);
    ui_->labelEditingTarget->setText(QStringLiteral("Editando entorno: %1")
                                              .arg(QString::fromStdString(env->name)));
    ui_->btnBackToMainMap->setVisible(true);
    setMainOnlySectionsVisible(false);
    selectTool(EditorTool::None);
}

void EditorWindow::setMainOnlySectionsVisible(bool visible) {
    ui_->labelCity->setVisible(visible);
    ui_->comboCityTemplate->setVisible(visible);
    ui_->btnToolCity->setVisible(visible);

    ui_->labelBiome->setVisible(visible);
    ui_->comboBiomeTemplate->setVisible(visible);
    ui_->btnToolBiome->setVisible(visible);

    ui_->labelEntry->setVisible(visible);
    ui_->comboEntryTemplate->setVisible(visible);
    ui_->btnToolEntry->setVisible(visible);

    ui_->labelEnvironments->setVisible(visible);
    ui_->listEnvironments->setVisible(visible);

    ui_->labelWall->setVisible(!visible);
    ui_->comboWallTemplate->setVisible(!visible);
    ui_->btnToolWall->setVisible(!visible);
}

void EditorWindow::refreshEnvironmentsList() {
    ui_->listEnvironments->clear();
    for (const auto& env: main_doc_.environments) {
        const QString label = QStringLiteral("%1 (%2 - %3x%4)")
                                      .arg(QString::fromStdString(env.name))
                                      .arg(QString::fromStdString(env.type))
                                      .arg(env.width)
                                      .arg(env.height);
        auto* item = new QListWidgetItem(label, ui_->listEnvironments);
        item->setData(Qt::UserRole, QString::fromStdString(env.id));
    }
}

Environment* EditorWindow::find_environment(const QString& id) {
    for (auto& env: main_doc_.environments) {
        if (QString::fromStdString(env.id) == id) {
            return &env;
        }
    }
    return nullptr;
}

void EditorWindow::saveMap() {
    saveCurrentToDocument();

    Verificator verificator(main_doc_);
    QString error_title;
    QString error_message;
    if (!verificator.validate(error_title, error_message)) {
        QMessageBox::warning(this, error_title, error_message);
        return;
    }

    const QString path = QStringLiteral("%1/%2.yaml")
                                 .arg(SAVE_MAP, QString::fromStdString(main_doc_.map.id));
    if (!YamlMapIO::save(main_doc_, path.toStdString())) {
        QMessageBox::warning(this, QStringLiteral("Error"),
                             QStringLiteral("No se pudo guardar el YAML."));
        return;
    }

    QMessageBox::information(this, QStringLiteral("Guardado"),
                             QStringLiteral("Mapa guardado en:\n%1").arg(path));
}
