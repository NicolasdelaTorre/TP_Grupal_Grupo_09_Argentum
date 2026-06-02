#include "editor_window.h"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QSpinBox>
#include <algorithm>

#include "dialogs/new_environment_dialog.h"
#include "map/yaml_map_io.h"

#include "editor_constants.h"
#include "ui_EditorWindow.h"
#include "verificator.h"

EditorWindow::ResizeDelta EditorWindow::computeResizeDelta(ResizeDirection dir, int cells,
                                                           bool shrink) {
    const int signed_cells = shrink ? -cells : cells;
    ResizeDelta r;
    switch (dir) {
    case ResizeDirection::Right:
        r.delta_w = signed_cells;
        break;
    case ResizeDirection::Down:
        r.delta_h = signed_cells;
        break;
    case ResizeDirection::Left:
        r.delta_w = signed_cells;
        r.offset_x = signed_cells;
        break;
    case ResizeDirection::Up:
        r.delta_h = signed_cells;
        r.offset_y = signed_cells;
        break;
    }
    return r;
}

bool EditorWindow::fitsInside(int x, int y, int w, int h, int map_w, int map_h) {
    return x >= 0 && y >= 0 && (x + w) <= map_w && (y + h) <= map_h;
}

bool EditorWindow::canShrinkDocument(const MapDocument& doc, const ResizeDelta& delta) {
    const int new_w = doc.map.width + delta.delta_w;
    const int new_h = doc.map.height + delta.delta_h;
    if (new_w <= 0 || new_h <= 0) {
        return false;
    }
    if (doc.player_spawn.placed) {
        const int x = doc.player_spawn.x + delta.offset_x;
        const int y = doc.player_spawn.y + delta.offset_y;
        if (!fitsInside(x, y, 1, 1, new_w, new_h)) {
            return false;
        }
    }
    for (const auto& o: doc.obstacles) {
        if (!fitsInside(o.x + delta.offset_x, o.y + delta.offset_y, o.width, o.height, new_w,
                        new_h)) {
            return false;
        }
    }
    for (const auto& w: doc.walls) {
        if (!fitsInside(w.x + delta.offset_x, w.y + delta.offset_y, w.width, w.height, new_w,
                        new_h)) {
            return false;
        }
    }
    for (const auto& e: doc.entries) {
        if (!fitsInside(e.x + delta.offset_x, e.y + delta.offset_y, e.width, e.height, new_w,
                        new_h)) {
            return false;
        }
    }
    for (const auto& z: doc.zones) {
        if (!fitsInside(z.area_x + delta.offset_x, z.area_y + delta.offset_y, z.area_width,
                        z.area_height, new_w, new_h)) {
            return false;
        }
        for (const auto& npc: z.fixed_npcs) {
            const int x = npc.x + delta.offset_x;
            const int y = npc.y + delta.offset_y;
            if (!fitsInside(x, y, 1, 1, new_w, new_h)) {
                return false;
            }
        }
    }
    return true;
}

void EditorWindow::applyResizeToDocument(MapDocument& doc, const ResizeDelta& delta) {
    doc.map.width += delta.delta_w;
    doc.map.height += delta.delta_h;

    if (delta.offset_x == 0 && delta.offset_y == 0) {
        return;
    }
    if (doc.player_spawn.placed) {
        doc.player_spawn.x += delta.offset_x;
        doc.player_spawn.y += delta.offset_y;
    }
    for (auto& o: doc.obstacles) {
        o.x += delta.offset_x;
        o.y += delta.offset_y;
    }
    for (auto& w: doc.walls) {
        w.x += delta.offset_x;
        w.y += delta.offset_y;
    }
    for (auto& e: doc.entries) {
        e.x += delta.offset_x;
        e.y += delta.offset_y;
    }
    for (auto& z: doc.zones) {
        z.area_x += delta.offset_x;
        z.area_y += delta.offset_y;
        for (auto& npc: z.fixed_npcs) {
            npc.x += delta.offset_x;
            npc.y += delta.offset_y;
        }
    }
}

EditorWindow::EditorWindow(QWidget* parent):
        QMainWindow(parent), ui_(new Ui::EditorWindow), tool_group_(new QButtonGroup(this)) {
    ui_->setupUi(this);

    {
        QPixmap logo_argentum(QStringLiteral(":/ui/logo_argentum.png"));
        QPixmap logo_map_editor(QStringLiteral(":/ui/logo_map_editor.png"));
        if (!logo_argentum.isNull() && !logo_map_editor.isNull()) {
            const QPixmap arg_scaled =
                    logo_argentum.scaledToWidth(500, Qt::SmoothTransformation);
            const QPixmap me_scaled =
                    logo_map_editor.scaledToWidth(380, Qt::SmoothTransformation);

            const int overlap_px = 50;
            const int total_w = std::max(arg_scaled.width(), me_scaled.width());
            const int total_h = arg_scaled.height() + me_scaled.height() - overlap_px;

            QPixmap composite(total_w, total_h);
            composite.fill(Qt::transparent);
            QPainter painter(&composite);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.drawPixmap((total_w - arg_scaled.width()) / 2, 0, arg_scaled);
            painter.drawPixmap((total_w - me_scaled.width()) / 2,
                               arg_scaled.height() - overlap_px, me_scaled);
            painter.end();

            ui_->labelLogoArgentum->setPixmap(composite);
        }
        ui_->labelLogoMapEditor->setVisible(false);
    }

    auto back_to_main_policy = ui_->btnBackToMainMap->sizePolicy();
    back_to_main_policy.setRetainSizeWhenHidden(true);
    ui_->btnBackToMainMap->setSizePolicy(back_to_main_policy);

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

    connect(ui_->btnBackToMainMap, &QPushButton::clicked, this, [this]() { backToMainMap(); });

    connect(ui_->listEnvironments, &QListWidget::itemDoubleClicked, this,
            &EditorWindow::onEnvironmentDoubleClicked);

    connect(map_canvas_, &MapCanvas::entryPlacementRequested, this,
            &EditorWindow::onEntryPlacementRequested);
    connect(map_canvas_, &MapCanvas::entryDeleted, this, &EditorWindow::onEntryDeleted);
    connect(map_canvas_, &MapCanvas::saveRequested, this, &EditorWindow::saveMap);
<<<<<<< HEAD
    connect(map_canvas_, &MapCanvas::biomeHoverInfo, ui_->labelBiomeHoverSpawns,
            &QLabel::setText);

    connect(ui_->btnApplyResize, &QPushButton::clicked, this, &EditorWindow::onApplyMapResize);

    auto* resize_action_group = new QButtonGroup(this);
    resize_action_group->setExclusive(true);
    resize_action_group->addButton(ui_->btnResizeExpand);
    resize_action_group->addButton(ui_->btnResizeShrink);

    auto* resize_direction_group = new QButtonGroup(this);
    resize_direction_group->setExclusive(true);
    resize_direction_group->addButton(ui_->btnResizeUp);
    resize_direction_group->addButton(ui_->btnResizeDown);
    resize_direction_group->addButton(ui_->btnResizeLeft);
    resize_direction_group->addButton(ui_->btnResizeRight);

    ui_->btnResizeExpand->setChecked(true);
    ui_->btnResizeRight->setChecked(true);
=======
    connect(map_canvas_, &MapCanvas::biomeHoverInfo, ui_->labelBiomeHoverSpawns, &QLabel::setText);
>>>>>>> origin/main
}

EditorWindow::~EditorWindow() { delete ui_; }

void EditorWindow::setupTemplates() {
    for (const auto& biome: templates_.biomes()) {
        auto* item =
                new QListWidgetItem(QString::fromStdString(biome.name), ui_->listBiomeTemplate);
        item->setData(Qt::UserRole, QString::fromStdString(biome.id));
    }
    if (ui_->listBiomeTemplate->count() > 0) {
        ui_->listBiomeTemplate->setCurrentRow(0);
    }
    for (const auto& city: templates_.cities()) {
        auto* item = new QListWidgetItem(QString::fromStdString(city.name), ui_->listCityTemplate);
        item->setData(Qt::UserRole, QString::fromStdString(city.id));
    }
    if (ui_->listCityTemplate->count() > 0) {
        ui_->listCityTemplate->setCurrentRow(0);
    }
    for (const auto& obstacle: templates_.obstacles()) {
        const QString label = QStringLiteral("%1 (%2x%3)")
                                      .arg(QString::fromStdString(obstacle.name))
                                      .arg(obstacle.width)
                                      .arg(obstacle.height);
        auto* item = new QListWidgetItem(label, ui_->listObstacleTemplate);
        item->setData(Qt::UserRole, QString::fromStdString(obstacle.id));
    }
    if (ui_->listObstacleTemplate->count() > 0) {
        ui_->listObstacleTemplate->setCurrentRow(0);
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
    tool_group_->addButton(ui_->btnModeSpawn);
    tool_group_->addButton(ui_->btnModeObstacles);
    tool_group_->addButton(ui_->btnModeBiomes);
    tool_group_->addButton(ui_->btnModeCities);
    tool_group_->addButton(ui_->btnModeEnvironments);
    tool_group_->addButton(ui_->btnModeDimensions);

    connect(ui_->btnModeSpawn, &QPushButton::clicked, this, &EditorWindow::selectSpawnMode);
    connect(ui_->btnModeObstacles, &QPushButton::clicked, this, &EditorWindow::selectObstacleMode);
    connect(ui_->btnModeBiomes, &QPushButton::clicked, this, &EditorWindow::selectBiomeMode);
    connect(ui_->btnModeCities, &QPushButton::clicked, this, &EditorWindow::selectCityMode);
    connect(ui_->btnModeEnvironments, &QPushButton::clicked, this,
            &EditorWindow::selectEnvironmentMode);
    connect(ui_->btnModeDimensions, &QPushButton::clicked, this,
            &EditorWindow::selectDimensionsMode);

    connect(ui_->listObstacleTemplate, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem*, QListWidgetItem*) { applyActiveTool(); });
    connect(ui_->listBiomeTemplate, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem*, QListWidgetItem*) { applyActiveTool(); });
    connect(ui_->listCityTemplate, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem*, QListWidgetItem*) { applyActiveTool(); });
    connect(ui_->comboEntryTemplate, &QComboBox::currentIndexChanged, this,
            [this](int) { applyActiveTool(); });
    connect(ui_->comboWallTemplate, &QComboBox::currentIndexChanged, this,
            [this](int) { applyActiveTool(); });

    selectDefaultMode();
}

void EditorWindow::selectTool(EditorTool tool) {
    active_tool_.tool = tool;
    applyActiveTool();
}

void EditorWindow::applyActiveTool() {
    if (auto* item = ui_->listObstacleTemplate->currentItem()) {
        active_tool_.obstacle_template_id = item->data(Qt::UserRole).toString();
    } else {
        active_tool_.obstacle_template_id.clear();
    }
    if (auto* item = ui_->listBiomeTemplate->currentItem()) {
        active_tool_.biome_template_id = item->data(Qt::UserRole).toString();
    } else {
        active_tool_.biome_template_id.clear();
    }
    if (auto* item = ui_->listCityTemplate->currentItem()) {
        active_tool_.city_template_id = item->data(Qt::UserRole).toString();
    } else {
        active_tool_.city_template_id.clear();
    }
    active_tool_.entry_template_id = ui_->comboEntryTemplate->currentData().toString();
    active_tool_.wall_template_id = ui_->comboWallTemplate->currentData().toString();
    map_canvas_->setActiveTool(active_tool_);
}

void EditorWindow::selectSpawnMode() {
    ui_->toolsStack->setCurrentWidget(ui_->pageToolSpawn);
    selectTool(EditorTool::PlayerSpawn);
}

void EditorWindow::selectObstacleMode() {
    ui_->toolsStack->setCurrentWidget(ui_->pageToolObstacles);
    selectTool(EditorTool::Obstacle);
}

void EditorWindow::selectBiomeMode() {
    ui_->toolsStack->setCurrentWidget(ui_->pageToolBiomes);
    ui_->labelBiomeHoverSpawns->clear();
    selectTool(EditorTool::BiomeZone);
}

void EditorWindow::selectCityMode() {
    ui_->toolsStack->setCurrentWidget(ui_->pageToolCities);
    selectTool(EditorTool::CityZone);
}

void EditorWindow::selectEnvironmentMode() {
    if (map_canvas_->editing_mode() == EditingMode::MainMap) {
        ui_->toolsStack->setCurrentWidget(ui_->pageToolEnvironments);
        selectTool(EditorTool::Entry);
        return;
    }

    ui_->toolsStack->setCurrentWidget(ui_->pageToolWalls);
    selectTool(EditorTool::Wall);
}

void EditorWindow::selectDimensionsMode() {
    ui_->toolsStack->setCurrentWidget(ui_->pageToolDimensions);
    selectTool(EditorTool::None);
    updateDimensionsLabel();
}

void EditorWindow::selectDefaultMode() {
    ui_->btnModeSpawn->setChecked(true);
    selectSpawnMode();
}

void EditorWindow::updateDimensionsLabel() {
<<<<<<< HEAD
    ui_->labelMapDimensions->setText(QStringLiteral("Map Size: %1 x %2")
                                             .arg(map_canvas_->map_width())
                                             .arg(map_canvas_->map_height()));
=======
    ui_->labelMapDimensions->setText(
            QStringLiteral("%1 x %2").arg(map_canvas_->map_width()).arg(map_canvas_->map_height()));
>>>>>>> origin/main
}

void EditorWindow::onApplyMapResize() {
    ResizeDirection dir;
    if (ui_->btnResizeUp->isChecked()) {
        dir = ResizeDirection::Up;
    } else if (ui_->btnResizeDown->isChecked()) {
        dir = ResizeDirection::Down;
    } else if (ui_->btnResizeLeft->isChecked()) {
        dir = ResizeDirection::Left;
    } else if (ui_->btnResizeRight->isChecked()) {
        dir = ResizeDirection::Right;
    } else {
        QMessageBox::warning(this, QStringLiteral("Resize"),
                             QStringLiteral("Choose a direction."));
        return;
    }

    const bool shrink = ui_->btnResizeShrink->isChecked();
    if (!shrink && !ui_->btnResizeExpand->isChecked()) {
        QMessageBox::warning(this, QStringLiteral("Resize"),
                             QStringLiteral("Choose an action (Expand or Reduce)."));
        return;
    }

    const int cells = ui_->spinResizeCells->value();
    if (cells <= 0) {
        return;
    }

    constexpr int MAX_DIM = 2000;
    constexpr int MIN_DIM = 1;
    saveCurrentToDocument();

    const ResizeDelta delta = computeResizeDelta(dir, cells, shrink);

    auto applyTo = [&](MapDocument& doc, const QString& context) -> bool {
        const int target_w = doc.map.width + delta.delta_w;
        const int target_h = doc.map.height + delta.delta_h;

        if (!shrink) {
            if (target_w > MAX_DIM || target_h > MAX_DIM) {
                QMessageBox::warning(this, QStringLiteral("Resize"),
                                     QStringLiteral("New size exceeds the %1 cells limit for %2.")
                                             .arg(MAX_DIM)
                                             .arg(context));
                return false;
            }
            applyResizeToDocument(doc, delta);
            return true;
        }

        if (target_w < MIN_DIM || target_h < MIN_DIM) {
            QMessageBox::warning(this, QStringLiteral("Resize"),
                                 QStringLiteral("Cant reduce map size with objects interfering"));
            return false;
        }
        if (!canShrinkDocument(doc, delta)) {
            QMessageBox::warning(this, QStringLiteral("Resize"),
                                 QStringLiteral("Cant reduce map size with objects interfering"));
            return false;
        }
        applyResizeToDocument(doc, delta);
        return true;
    };

    if (map_canvas_->editing_mode() == EditingMode::MainMap) {
        if (!applyTo(main_doc_, QStringLiteral("the main map"))) {
            return;
        }
        map_canvas_->loadFromDocument(main_doc_, EditingMode::MainMap);
    } else {
        Environment* env = find_environment(current_environment_id_);
        if (!env) {
            return;
        }

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

        if (!applyTo(env_doc, QStringLiteral("the environment"))) {
            return;
        }

        env->width = env_doc.map.width;
        env->height = env_doc.map.height;
        env->player_spawn = env_doc.player_spawn;
        env->obstacles = env_doc.obstacles;
        env->walls = env_doc.walls;

        map_canvas_->loadFromDocument(env_doc, EditingMode::Environment);
    }

    updateDimensionsLabel();
    refreshEnvironmentsList();
}

void EditorWindow::setupNewMapPage() {
    ui_->comboNewMapSize->clear();
    ui_->comboNewMapSize->addItem(QStringLiteral("100 x 100"), QSize(100, 100));
    ui_->comboNewMapSize->addItem(QStringLiteral("250 x 250"), QSize(250, 250));
    ui_->comboNewMapSize->addItem(QStringLiteral("500 x 500"), QSize(500, 500));
}

void EditorWindow::resetNewMapPage() {
    ui_->inputNewMapId->setText(QStringLiteral("otro_mapa"));
    ui_->inputNewMapName->setText(QStringLiteral("Otro mapa"));
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
    updateDimensionsLabel();
    selectDefaultMode();
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
    updateDimensionsLabel();
    ui_->btnModeEnvironments->setChecked(true);
    selectEnvironmentMode();
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
    ui_->labelEditingTarget->setText(
            QStringLiteral("Editando entorno: %1").arg(QString::fromStdString(env->name)));
    ui_->btnBackToMainMap->setVisible(true);
    setMainOnlySectionsVisible(false);
    updateDimensionsLabel();
    selectDefaultMode();
}

void EditorWindow::setMainOnlySectionsVisible(bool visible) {
    ui_->btnModeBiomes->setVisible(visible);
    ui_->btnModeCities->setVisible(visible);
    ui_->btnModeDimensions->setVisible(true);
    ui_->btnModeEnvironments->setText(visible ? QStringLiteral("Environments") :
                                                QStringLiteral("Walls"));
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

    const QString path =
            QStringLiteral("%1/%2.yaml").arg(SAVE_MAP, QString::fromStdString(main_doc_.map.id));
    if (!YamlMapIO::save(main_doc_, path.toStdString())) {
        QMessageBox::warning(this, QStringLiteral("Error"),
                             QStringLiteral("No se pudo guardar el YAML."));
        return;
    }

    QMessageBox::information(this, QStringLiteral("Guardado"),
                             QStringLiteral("Mapa guardado en:\n%1").arg(path));
}
