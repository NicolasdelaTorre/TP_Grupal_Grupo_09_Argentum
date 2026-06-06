#include "map_canvas.h"

#include <QColor>
#include <QCoreApplication>
#include <QFileInfo>
#include <QGraphicsRectItem>
#include <QHBoxLayout>
#include <QImage>
#include <QInputDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

#include "dialogs/biome_spawn_dialog.h"
#include "map/biome_grid.h"

#include "editor_constants.h"

MapCanvas::MapCanvas(const TemplateRegistry& templates, QWidget* parent):
        QWidget(parent),
        templates_(templates),
        scene_(new QGraphicsScene(this)),
        view_(new QGraphicsView(this)),
        controller_(new SceneController(scene_, templates)) {
    // layout de la ventana
    auto* layout = new QVBoxLayout(this);
    // vista de la escena
    view_->setRenderHint(QPainter::Antialiasing);
    view_->setDragMode(QGraphicsView::ScrollHandDrag);
    view_->setScene(scene_);
    view_->viewport()->installEventFilter(this);
    view_->installEventFilter(this);
    view_->setMouseTracking(true);
    view_->viewport()->setMouseTracking(true);
    // añadir vista al layout
    layout->addWidget(view_);
    // botones guardar, zoom in y zoom out
    auto* save_button = new QPushButton(QStringLiteral("Guardar mapa"), this);
    save_button->setProperty("primary", true);
    auto* zoom_in_button = new QPushButton(QStringLiteral("+"), this);
    auto* zoom_out_button = new QPushButton(QStringLiteral("–"), this);
    // layout de botones
    auto* buttons = new QHBoxLayout();
    buttons->addWidget(save_button);
    buttons->addWidget(zoom_in_button);
    buttons->addWidget(zoom_out_button);
    layout->addLayout(buttons);
    // conectar botones a funciones
    connect(save_button, &QPushButton::clicked, this, &MapCanvas::onSaveClicked);
    connect(zoom_in_button, &QPushButton::clicked, this, [this]() { zoomIn(); });
    connect(zoom_out_button, &QPushButton::clicked, this, [this]() { zoomOut(); });
}

void MapCanvas::onSaveClicked() { emit saveRequested(); }

void MapCanvas::initializeScene(const QString& map_id, const QString& map_name, int width,
                                int height) {
    map_id_ = map_id;
    map_name_ = map_name;
    map_width_ = width;
    map_height_ = height;

    scene_->clear();
    controller_->reset();
    drawing_zone_ = false;
    clearZonePreview();
    biome_tint_item_ = nullptr;
    // scene_->clear() ya destruyó los items de textura; los punteros del vector quedaron
    // dangling, así que limpiamos el vector (no hace falta llamar delete).
    biome_texture_items_.clear();
    env_floor_item_ = nullptr;
    env_exterior_item_ = nullptr;
    // Ancho y alto de la escena
    const int scene_width = width * CELL_DISPLAY_SIZE;
    const int scene_height = height * CELL_DISPLAY_SIZE;

    auto* background = scene_->addRect(0, 0, scene_width, scene_height, QPen(Qt::darkGray),
                                       QBrush(QColor(235, 235, 220)));
    background->setZValue(-2);
    // capa de tinte por bioma: pixmap de 1 px por celda escalado por CELL_DISPLAY_SIZE
    biome_tint_item_ = scene_->addPixmap(QPixmap());
    biome_tint_item_->setTransformationMode(Qt::FastTransformation);
    biome_tint_item_->setScale(CELL_DISPLAY_SIZE);
    biome_tint_item_->setZValue(-1.5);
    // capas de entorno: piso (rect texturizado tileable) y overlay negro encima del piso.
    env_floor_item_ =
            scene_->addRect(0, 0, scene_width, scene_height, QPen(Qt::NoPen), Qt::NoBrush);
    env_floor_item_->setZValue(-1.8);
    env_floor_item_->setVisible(false);
    env_exterior_item_ = scene_->addPixmap(QPixmap());
    env_exterior_item_->setTransformationMode(Qt::FastTransformation);
    env_exterior_item_->setScale(CELL_DISPLAY_SIZE);
    env_exterior_item_->setZValue(-1.6);
    env_exterior_item_->setVisible(false);
    drawGrid();
    scene_->setSceneRect(0, 0, scene_width, scene_height);
    QTimer::singleShot(0, this, [this]() { applyInitialView(); });
}

void MapCanvas::createMap(const QString& map_id, const QString& map_name, int width, int height) {
    editing_mode_ = EditingMode::MainMap;
    initializeScene(map_id, map_name, width, height);
    emit statusMessage(QStringLiteral("Mapa %1 (%2x%3)").arg(map_name).arg(width).arg(height));
}

void MapCanvas::applyInitialView() {
    view_->resetTransform();
    // ancho y alto de la escena
    const int scene_width = map_width_ * CELL_DISPLAY_SIZE;
    const int scene_height = map_height_ * CELL_DISPLAY_SIZE;

    // ancho y alto de la ventana
    const int viewport_w = std::max(view_->viewport()->width(), 1);
    const int viewport_h = std::max(view_->viewport()->height(), 1);

    // escala de la vista para que entre en la ventana
    const double fit_scale_x = (viewport_w * 0.92) / scene_width;
    const double fit_scale_y = (viewport_h * 0.92) / scene_height;
    const double fit_scale = std::min(fit_scale_x, fit_scale_y);

    // escala de la vista para que cada celda ocupe el tamaño TARGET_CELL_SCREEN_PX
    const double target_scale = static_cast<double>(TARGET_CELL_SCREEN_PX) / CELL_DISPLAY_SIZE;
    // escala de la vista para que entre en la ventana y cada celda ocupe el tamaño
    // TARGET_CELL_SCREEN_PX
    const double scale = std::max(fit_scale, target_scale);

    // escalar vista
    view_->scale(scale, scale);
    // escalar zoom
    current_zoom_ = scale;
    view_->centerOn(scene_width / 2.0, scene_height / 2.0);
}

void MapCanvas::drawGrid() {
    QPen grid_pen(QColor(170, 170, 170));
    grid_pen.setWidth(1);
    grid_pen.setCosmetic(true);

    const int scene_width = map_width_ * CELL_DISPLAY_SIZE;
    const int scene_height = map_height_ * CELL_DISPLAY_SIZE;

    for (int x = 0; x <= scene_width; x += CELL_DISPLAY_SIZE) {
        scene_->addLine(x, 0, x, scene_height, grid_pen)->setZValue(-1);
    }
    for (int y = 0; y <= scene_height; y += CELL_DISPLAY_SIZE) {
        scene_->addLine(0, y, scene_width, y, grid_pen)->setZValue(-1);
    }
}
void MapCanvas::setActiveTool(const ToolInfo& tool) {
    if (!isToolAllowed(tool.tool)) {
        active_tool_ = ToolInfo();
        return;
    }
    active_tool_ = tool;
}

QString MapCanvas::map_id() const { return map_id_; }

QString MapCanvas::map_name() const { return map_name_; }

int MapCanvas::map_width() const { return map_width_; }

int MapCanvas::map_height() const { return map_height_; }

EditingMode MapCanvas::editing_mode() const { return editing_mode_; }

bool MapCanvas::isToolAllowed(EditorTool tool) const {
    if (editing_mode_ == EditingMode::MainMap) {
        return tool != EditorTool::Wall;
    }
    return tool == EditorTool::None || tool == EditorTool::Obstacle ||
           tool == EditorTool::PlayerSpawn || tool == EditorTool::Wall;
}

MapDocument MapCanvas::buildDocument() const {
    return controller_->buildDocument(map_id_, map_name_, map_width_, map_height_);
}

void MapCanvas::loadFromDocument(const MapDocument& document, EditingMode mode) {
    editing_mode_ = mode;
    env_floor_color_ = QString::fromStdString(document.floor_color);
    initializeScene(QString::fromStdString(document.map.id),
                    QString::fromStdString(document.map.name), document.map.width,
                    document.map.height);

    QString error;
    if (document.player_spawn.placed) {
        controller_->placePlayerSpawn(document.player_spawn.x, document.player_spawn.y, error);
    }

    for (const auto& obstacle: document.obstacles) {
        ToolInfo tool;
        tool.tool = EditorTool::Obstacle;
        tool.obstacle_template_id = QString::fromStdString(obstacle.type);
        controller_->placeObstacle(tool, obstacle.x, obstacle.y, error);
    }

    for (const auto& zone: document.zones) {
        ToolInfo tool;
        if (zone.type == ZONE_TYPE_CITY) {
            tool.tool = EditorTool::CityZone;
            tool.city_template_id = QString::fromStdString(zone.template_id);
            controller_->placeCityZone(tool, zone.area_x, zone.area_y, zone.area_width,
                                       zone.area_height, error, QString::fromStdString(zone.id));
        } else if (zone.type == ZONE_TYPE_BIOME) {
            tool.tool = EditorTool::BiomeZone;
            tool.biome_template_id = QString::fromStdString(zone.template_id);
            controller_->placeBiomeZone(tool, zone.area_x, zone.area_y, zone.area_width,
                                        zone.area_height, zone.spawns, error,
                                        QString::fromStdString(zone.id));
        }
    }

    for (const auto& entry: document.entries) {
        controller_->placeEntry(QString::fromStdString(entry.id),
                                QString::fromStdString(entry.environment_id),
                                QString::fromStdString(entry.type), entry.x, entry.y, error);
    }

    for (const auto& wall: document.walls) {
        ToolInfo tool;
        tool.tool = EditorTool::Wall;
        tool.wall_template_id = QString::fromStdString(wall.template_id);
        controller_->placeWall(tool, wall.x, wall.y, error, QString::fromStdString(wall.id));
    }

    rebuildBiomeTint();
    rebuildEnvironmentLayers();

    const QString header = (mode == EditingMode::MainMap) ? QStringLiteral("Mapa %1 (%2x%3)") :
                                                            QStringLiteral("Entorno %1 (%2x%3)");
    emit statusMessage(header.arg(map_name_).arg(map_width_).arg(map_height_));
}

bool MapCanvas::placeEntryItem(const QString& entry_id, const QString& environment_id,
                               const QString& template_id, int cell_x, int cell_y) {
    QString error;
    if (!controller_->placeEntry(entry_id, environment_id, template_id, cell_x, cell_y, error)) {
        QMessageBox::warning(this, QStringLiteral("Entrada"), error);
        return false;
    }
    return true;
}
// convertir posición de la vista a la posición de la celda
void MapCanvas::cellFromViewPos(const QPoint& view_pos, int& cell_x, int& cell_y) const {
    const QPointF scene_pos = view_->mapToScene(view_pos);
    cell_x = static_cast<int>(scene_pos.x()) / CELL_DISPLAY_SIZE;
    cell_y = static_cast<int>(scene_pos.y()) / CELL_DISPLAY_SIZE;
}

QRect MapCanvas::normalizedCellRect(const QPoint& a, const QPoint& b) const {
    const int x1 = std::min(a.x(), b.x());
    const int y1 = std::min(a.y(), b.y());
    const int x2 = std::max(a.x(), b.x());
    const int y2 = std::max(a.y(), b.y());
    return QRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

void MapCanvas::clearZonePreview() {
    if (zone_preview_) {
        scene_->removeItem(zone_preview_);
        delete zone_preview_;
        zone_preview_ = nullptr;
    }
}
// manejar click izquierdo
void MapCanvas::handleLeftPress(const QPoint& view_pos) {
    int cell_x = 0;
    int cell_y = 0;
    cellFromViewPos(view_pos, cell_x, cell_y);
    // si celda está fuera del mapa, no hacer nada
    if (cell_x < 0 || cell_y < 0 || cell_x >= map_width_ || cell_y >= map_height_) {
        return;
    }

    if (!isToolAllowed(active_tool_.tool)) {
        return;
    }

    QString error;
    // poner respecitvo item
    switch (active_tool_.tool) {
        case EditorTool::PlayerSpawn:
            controller_->placePlayerSpawn(cell_x, cell_y, error);
            break;
        case EditorTool::Obstacle:
            placeObstacleAt(cell_x, cell_y);
            break;
        case EditorTool::CityZone:
            placeCityAt(cell_x, cell_y);
            break;
        case EditorTool::BiomeZone:
            if (!drawing_zone_) {
                drawing_zone_ = true;
                zone_start_cell_ = QPoint(cell_x, cell_y);
                clearZonePreview();
            }
            break;
        case EditorTool::Entry:
            requestEntryAt(cell_x, cell_y);
            break;
        case EditorTool::Wall:
            placeWallAt(cell_x, cell_y);
            break;
        default:
            break;
    }
}

// pedir a EditorWindow que muestre el diálogo y luego confirme la entrada con un id de entorno.
void MapCanvas::requestEntryAt(int cell_x, int cell_y) {
    const auto* entry = templates_.find_entry(active_tool_.entry_template_id.toStdString());
    if (!entry) {
        QMessageBox::warning(this, QStringLiteral("Entrada"),
                             QStringLiteral("Template de entrada inválido."));
        return;
    }

    if (cell_x + entry->width > map_width_ || cell_y + entry->height > map_height_) {
        QMessageBox::warning(this, QStringLiteral("Entrada"),
                             QStringLiteral("La entrada no entra en el mapa desde esa posición."));
        return;
    }

    emit entryPlacementRequested(active_tool_.entry_template_id, cell_x, cell_y);
}
// manejar movimiento del mouse
void MapCanvas::handleMouseMove(const QPoint& view_pos) {
    if (!drawing_zone_) {
        return;
    }
    // convertir posición de la vista a la posición de la celda
    int cell_x = 0;
    int cell_y = 0;
    cellFromViewPos(view_pos, cell_x, cell_y);
    cell_x = std::clamp(cell_x, 0, map_width_ - 1);
    cell_y = std::clamp(cell_y, 0, map_height_ - 1);

    const QRect rect = normalizedCellRect(zone_start_cell_, QPoint(cell_x, cell_y));
    clearZonePreview();
    zone_preview_ =
            scene_->addRect(rect.x() * CELL_DISPLAY_SIZE, rect.y() * CELL_DISPLAY_SIZE,
                            rect.width() * CELL_DISPLAY_SIZE, rect.height() * CELL_DISPLAY_SIZE,
                            QPen(Qt::DashLine), QBrush(QColor(255, 255, 0, 60)));
    zone_preview_->setZValue(Z_ZONE_PREVIEW);
}

// hover sobre un bioma existente: emitir info de spawns para el panel lateral.
void MapCanvas::handleHoverMove(const QPoint& view_pos) {
    if (active_tool_.tool != EditorTool::BiomeZone) {
        if (!last_hover_zone_id_.isEmpty()) {
            last_hover_zone_id_.clear();
            emit biomeHoverInfo(QString());
        }
        return;
    }

    int cell_x = 0;
    int cell_y = 0;
    cellFromViewPos(view_pos, cell_x, cell_y);
    if (cell_x < 0 || cell_y < 0 || cell_x >= map_width_ || cell_y >= map_height_) {
        if (!last_hover_zone_id_.isEmpty()) {
            last_hover_zone_id_.clear();
            emit biomeHoverInfo(QString());
        }
        return;
    }

    auto* item = biomeZoneAtCell(cell_x, cell_y);
    if (!item) {
        if (!last_hover_zone_id_.isEmpty()) {
            last_hover_zone_id_.clear();
            emit biomeHoverInfo(QString());
        }
        return;
    }

    const QString zone_id = item->data(DATA_ID).toString();
    if (zone_id == last_hover_zone_id_) {
        return;
    }
    last_hover_zone_id_ = zone_id;

    const QString template_id = item->data(DATA_SUBTYPE).toString();
    const auto* tpl = templates_.find_biome(template_id.toStdString());
    QString text;
    if (tpl) {
        text = QString::fromStdString(tpl->name);
        text += QStringLiteral("\n");
    }

    const auto spawns = controller_->biomeSpawnsFor(zone_id);
    if (spawns.empty()) {
        text += QStringLiteral("Sin spawns configurados.");
    } else {
        for (const auto& spawn: spawns) {
            text += QStringLiteral("• %1: %2\n")
                            .arg(QString::fromStdString(spawn.creature))
                            .arg(spawn.max_population);
        }
    }
    emit biomeHoverInfo(text);
}
// colocar ciudad en celda
void MapCanvas::placeCityAt(int cell_x, int cell_y) {
    const auto* city = templates_.find_city(active_tool_.city_template_id.toStdString());
    if (!city) {
        QMessageBox::warning(this, QStringLiteral("Ciudad"),
                             QStringLiteral("Template de ciudad inválido."));
        return;
    }

    if (cell_x + city->default_width > map_width_ || cell_y + city->default_height > map_height_) {
        QMessageBox::warning(this, QStringLiteral("Ciudad"),
                             QStringLiteral("La ciudad no entra en el mapa desde esa posición."));
        return;
    }

    QString error;
    if (!controller_->placeCityZone(active_tool_, cell_x, cell_y, city->default_width,
                                    city->default_height, error)) {
        QMessageBox::warning(this, QStringLiteral("Ciudad"), error);
        return;
    }

    // Los obstáculos fijos de la ciudad se materializan como obstáculos normales
    // (se ven en el editor y se guardan junto al resto). Al cargar un mapa ya
    // vienen en la lista de obstáculos, por eso esto sólo corre al colocar la
    // ciudad de forma interactiva.
    for (const auto& fixed: city->fixed_obstacles) {
        ToolInfo obstacle_tool;
        obstacle_tool.tool = EditorTool::Obstacle;
        obstacle_tool.obstacle_template_id = QString::fromStdString(fixed.type);
        QString obstacle_error;
        controller_->placeObstacle(obstacle_tool, cell_x + fixed.relative_x,
                                   cell_y + fixed.relative_y, obstacle_error);
    }
}

// colocar obstáculo en celda usando el template seleccionado
void MapCanvas::placeObstacleAt(int cell_x, int cell_y) {
    const auto* obstacle =
            templates_.find_obstacle(active_tool_.obstacle_template_id.toStdString());
    if (!obstacle) {
        QMessageBox::warning(this, QStringLiteral("Obstáculo"),
                             QStringLiteral("Template de obstáculo inválido."));
        return;
    }

    if (cell_x + obstacle->width > map_width_ || cell_y + obstacle->height > map_height_) {
        QMessageBox::warning(
                this, QStringLiteral("Obstáculo"),
                QStringLiteral("El obstáculo no entra en el mapa desde esa posición."));
        return;
    }

    QString error;
    if (!controller_->placeObstacle(active_tool_, cell_x, cell_y, error)) {
        QMessageBox::warning(this, QStringLiteral("Obstáculo"), error);
    }
}

// colocar pared en celda (click único, sin modo pintar)
void MapCanvas::placeWallAt(int cell_x, int cell_y) {
    const auto* wall = templates_.find_wall(active_tool_.wall_template_id.toStdString());
    if (!wall) {
        QMessageBox::warning(this, QStringLiteral("Pared"),
                             QStringLiteral("Template de pared inválido."));
        return;
    }

    if (cell_x + wall->width > map_width_ || cell_y + wall->height > map_height_) {
        QMessageBox::warning(this, QStringLiteral("Pared"),
                             QStringLiteral("La pared no entra en el mapa desde esa posición."));
        return;
    }

    QString error;
    if (!controller_->placeWall(active_tool_, cell_x, cell_y, error)) {
        QMessageBox::warning(this, QStringLiteral("Pared"), error);
        return;
    }
    rebuildEnvironmentLayers();
}

QGraphicsItem* MapCanvas::biomeZoneAtCell(int cell_x, int cell_y) const {
    const QRectF area(cell_x * CELL_DISPLAY_SIZE, cell_y * CELL_DISPLAY_SIZE, CELL_DISPLAY_SIZE,
                      CELL_DISPLAY_SIZE);
    const auto items = scene_->items(area);
    for (auto* item: items) {
        QGraphicsItem* current = item;
        while (current->parentItem()) {
            current = current->parentItem();
        }
        if (current->data(DATA_TYPE).toString() == BIOME_ZONE_TYPE) {
            return current;
        }
    }
    return nullptr;
}

void MapCanvas::editBiomeSpawnsAt(int cell_x, int cell_y) {
    auto* biome_item = biomeZoneAtCell(cell_x, cell_y);
    if (!biome_item) {
        return;
    }

    const QString template_id = biome_item->data(DATA_SUBTYPE).toString();
    const auto* biome = templates_.find_biome(template_id.toStdString());
    if (!biome) {
        QMessageBox::warning(this, QStringLiteral("Biome"),
                             QStringLiteral("Template de bioma inválido."));
        return;
    }

    const QString zone_id = biome_item->data(DATA_ID).toString();
    BiomeSpawnDialog dialog(*biome, controller_->biomeSpawnsFor(zone_id), this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    controller_->setBiomeSpawns(zone_id, dialog.selected_spawns());
    last_hover_zone_id_.clear();
}

// finalizar dibujo de zona de bioma
void MapCanvas::finishBiomeZoneDraw(int end_cell_x, int end_cell_y) {
    drawing_zone_ = false;
    clearZonePreview();
    end_cell_x = std::clamp(end_cell_x, 0, map_width_ - 1);
    end_cell_y = std::clamp(end_cell_y, 0, map_height_ - 1);
    // normalizar rectángulo de la celda
    QRect rect = normalizedCellRect(zone_start_cell_, QPoint(end_cell_x, end_cell_y));
    if (rect.width() == 1 && rect.height() == 1) {
        editBiomeSpawnsAt(rect.x(), rect.y());
        return;
    }

    const auto* biome = templates_.find_biome(active_tool_.biome_template_id.toStdString());
    if (!biome) {
        QMessageBox::warning(this, QStringLiteral("Biome"),
                             QStringLiteral("Template de bioma inválido."));
        return;
    }
    // si el bioma no tiene criaturas disponibles, saltear el diálogo
    std::vector<CreatureSpawn> spawns;
    if (!biome->allowed_creatures.empty()) {
        BiomeSpawnDialog dialog(*biome, this);
        if (dialog.exec() != QDialog::Accepted) {
            return;
        }
        spawns = dialog.selected_spawns();
    }
    // colocar zona de bioma
    QString error;
    if (!controller_->placeBiomeZone(active_tool_, rect.x(), rect.y(), rect.width(), rect.height(),
                                     spawns, error)) {
        QMessageBox::warning(this, QStringLiteral("Biome"), error);
        return;
    }
    rebuildBiomeTint();
}
// manejar click derecho
void MapCanvas::handleRightPress(const QPoint& view_pos) {
    if (drawing_zone_) {
        drawing_zone_ = false;
        clearZonePreview();
        return;
    }

    int cell_x = 0;
    int cell_y = 0;
    cellFromViewPos(view_pos, cell_x, cell_y);
    const DeletedItem deleted = controller_->deleteAtCell(cell_x, cell_y);
    if (deleted.deleted && deleted.type == ENTRY_TYPE && !deleted.environment_id.isEmpty()) {
        emit entryDeleted(deleted.environment_id);
    }
    if (deleted.deleted && deleted.id == last_hover_zone_id_) {
        last_hover_zone_id_.clear();
        emit biomeHoverInfo(QString());
    }
    rebuildBiomeTint();
    if (deleted.deleted && deleted.type == WALL_TYPE) {
        rebuildEnvironmentLayers();
    }
}
// manejar eventos del mouse
bool MapCanvas::eventFilter(QObject* obj, QEvent* event) {
    if (obj != view_->viewport()) {
        return QWidget::eventFilter(obj, event);
    }
    // manejar click izquierdo
    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() == Qt::LeftButton) {
            handleLeftPress(mouse_event->pos());
            return true;
        }
        if (mouse_event->button() == Qt::RightButton) {
            handleRightPress(mouse_event->pos());
            return true;
        }
    }
    // manejar movimiento del mouse
    if (event->type() == QEvent::MouseMove) {
        auto* mouse_event = static_cast<QMouseEvent*>(event);
        if (drawing_zone_) {
            handleMouseMove(mouse_event->pos());
            return true;
        }
        handleHoverMove(mouse_event->pos());
    }
    // suelto el click izquierdo
    if (event->type() == QEvent::MouseButtonRelease && drawing_zone_) {
        auto* mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() == Qt::LeftButton) {
            int cell_x = 0;
            int cell_y = 0;
            cellFromViewPos(mouse_event->pos(), cell_x, cell_y);
            finishBiomeZoneDraw(cell_x, cell_y);
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}
void MapCanvas::zoomIn() {
    double new_zoom = current_zoom_ + ZOOM_SCALE;
    if (new_zoom > MAX_ZOOM) {
        new_zoom = MAX_ZOOM;
    }
    const double factor = new_zoom / current_zoom_;
    if (factor != 1.0) {
        view_->scale(factor, factor);
        current_zoom_ = new_zoom;
    }
}

void MapCanvas::zoomOut() {
    double new_zoom = current_zoom_ - ZOOM_SCALE;
    if (new_zoom < MIN_ZOOM) {
        new_zoom = MIN_ZOOM;
    }
    const double factor = new_zoom / current_zoom_;
    if (factor != 1.0) {
        view_->scale(factor, factor);
        current_zoom_ = new_zoom;
    }
}

// pintar piso del entorno + overlay negro en celdas exteriores (flood fill desde los bordes).
void MapCanvas::rebuildEnvironmentLayers() {
    if (!env_floor_item_ || !env_exterior_item_) {
        return;
    }

    if (editing_mode_ != EditingMode::Environment) {
        env_floor_item_->setVisible(false);
        env_exterior_item_->setVisible(false);
        return;
    }

    const int W = map_width_;
    const int H = map_height_;
    if (W <= 0 || H <= 0) {
        return;
    }

    // pintar piso con color sólido (el color viene del template de entrada).
    env_floor_item_->setRect(0, 0, W * CELL_DISPLAY_SIZE, H * CELL_DISPLAY_SIZE);
    QColor floor_color(60, 60, 60);
    if (!env_floor_color_.isEmpty()) {
        QColor parsed(env_floor_color_);
        if (parsed.isValid()) {
            floor_color = parsed;
        }
    }
    env_floor_item_->setBrush(QBrush(floor_color));
    env_floor_item_->setPos(0, 0);
    env_floor_item_->setVisible(true);

    // detectar paredes
    std::vector<bool> is_wall(static_cast<size_t>(W) * H, false);
    for (auto* item: scene_->items()) {
        if (item->data(DATA_TYPE).toString() != WALL_TYPE) {
            continue;
        }
        const int wx = static_cast<int>(item->pos().x()) / CELL_DISPLAY_SIZE;
        const int wy = static_cast<int>(item->pos().y()) / CELL_DISPLAY_SIZE;
        const int ww = std::max(1, item->data(DATA_WIDTH).toInt());
        const int wh = std::max(1, item->data(DATA_HEIGHT).toInt());
        for (int dy = 0; dy < wh; ++dy) {
            for (int dx = 0; dx < ww; ++dx) {
                const int cx = wx + dx;
                const int cy = wy + dy;
                if (cx < 0 || cy < 0 || cx >= W || cy >= H) {
                    continue;
                }
                is_wall[static_cast<size_t>(cy) * W + cx] = true;
            }
        }
    }

    // flood fill desde los bordes para marcar exterior.
    std::vector<bool> is_exterior(static_cast<size_t>(W) * H, false);
    std::queue<std::pair<int, int>> queue;
    auto enqueue_if_open = [&](int x, int y) {
        if (x < 0 || y < 0 || x >= W || y >= H) {
            return;
        }
        const size_t idx = static_cast<size_t>(y) * W + x;
        if (is_wall[idx] || is_exterior[idx]) {
            return;
        }
        is_exterior[idx] = true;
        queue.emplace(x, y);
    };

    for (int x = 0; x < W; ++x) {
        enqueue_if_open(x, 0);
        enqueue_if_open(x, H - 1);
    }
    for (int y = 0; y < H; ++y) {
        enqueue_if_open(0, y);
        enqueue_if_open(W - 1, y);
    }

    const int dx4[] = {1, -1, 0, 0};
    const int dy4[] = {0, 0, 1, -1};
    while (!queue.empty()) {
        const auto [cx, cy] = queue.front();
        queue.pop();
        for (int k = 0; k < 4; ++k) {
            enqueue_if_open(cx + dx4[k], cy + dy4[k]);
        }
    }

    // overlay negro solo en celdas exteriores (no paredes y sin recinto).
    QImage overlay(W, H, QImage::Format_ARGB32_Premultiplied);
    overlay.fill(Qt::transparent);

    // contar interiores para decidir si pintar overlay (si no hay recinto cerrado, mostrar todo el
    // piso).
    bool has_interior = false;
    for (int y = 0; y < H && !has_interior; ++y) {
        for (int x = 0; x < W && !has_interior; ++x) {
            const size_t idx = static_cast<size_t>(y) * W + x;
            if (!is_wall[idx] && !is_exterior[idx]) {
                has_interior = true;
            }
        }
    }

    if (has_interior) {
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                if (is_exterior[static_cast<size_t>(y) * W + x]) {
                    overlay.setPixelColor(x, y, QColor(0, 0, 0, 255));
                }
            }
        }
    }

    env_exterior_item_->setPixmap(QPixmap::fromImage(overlay));
    env_exterior_item_->setPos(0, 0);
    env_exterior_item_->setVisible(true);
}

// pintar grilla expandiendo el color de cada bioma hasta chocar con otro
void MapCanvas::rebuildBiomeTint() {
    if (!biome_tint_item_ || map_width_ <= 0 || map_height_ <= 0) {
        return;
    }

    // Limpiar items de textura de la corrida anterior. Los items pertenecen a la escena, primero
    // los removemos y después delete
    for (auto* item: biome_texture_items_) {
        scene_->removeItem(item);
        delete item;
    }
    biome_texture_items_.clear();

    const int W = map_width_;
    const int H = map_height_;

    QImage img(W, H, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    // recolectar zonas de bioma, color y textura.
    struct BiomeZone {
        QColor color;
        QString texture_path;
    };
    std::vector<BiomeZone> zones;
    std::vector<BiomeSource> sources;
    for (auto* item: scene_->items()) {
        if (item->data(DATA_TYPE).toString() != BIOME_ZONE_TYPE) {
            continue;
        }
        BiomeSource src;
        src.x = static_cast<int>(item->pos().x()) / CELL_DISPLAY_SIZE;
        src.y = static_cast<int>(item->pos().y()) / CELL_DISPLAY_SIZE;
        src.width = item->data(DATA_WIDTH).toInt();
        src.height = item->data(DATA_HEIGHT).toInt();

        BiomeZone z;
        const auto* tpl = templates_.find_biome(item->data(DATA_SUBTYPE).toString().toStdString());
        z.color = QColor(QString::fromStdString(tpl->color));
        z.texture_path = QString::fromStdString(tpl->texture);

        sources.push_back(src);
        zones.push_back(z);
    }

    if (zones.empty()) {
        biome_tint_item_->setPixmap(QPixmap::fromImage(img));
        return;
    }

    // Dijkstra multi-fuente (mismo helper que usa el guardado del mapa).
    const std::vector<int> owner = computeBiomeOwners(W, H, sources);

    // Resolver textura por bioma. Las texturas vienen a 128x128 pero
    // nuestros tiles son 64x64, se recorta al esquina superior izquierda.
    std::vector<QPixmap> biome_pixmaps(zones.size());
    for (size_t i = 0; i < zones.size(); ++i) {
        const QString& path = zones[i].texture_path;
        if (path.isEmpty()) {
            continue;
        }
        QPixmap loaded;
        if (!loaded.load(path)) {
            continue;
        }
        const int crop_w = std::min(loaded.width(), CELL_DISPLAY_SIZE);
        const int crop_h = std::min(loaded.height(), CELL_DISPLAY_SIZE);
        biome_pixmaps[i] = loaded.copy(0, 0, crop_w, crop_h);
    }

    // Para celdas con textura instanciar un QGraphicsPixmapItem por celda, para celdas sin textura
    // tinte de color con alpha 90.
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            const int o = owner[static_cast<size_t>(y) * W + x];
            if (o < 0) {
                continue;
            }
            const QPixmap& pm = biome_pixmaps[o];
            if (!pm.isNull()) {
                auto* tile = scene_->addPixmap(pm);
                tile->setTransformationMode(Qt::SmoothTransformation);
                tile->setPos(x * CELL_DISPLAY_SIZE, y * CELL_DISPLAY_SIZE);
                // Por debajo del tinte de color (z=-1.5) y por encima del background
                // (z=-2), así no oculta ni pisa otras capas.
                tile->setZValue(-1.7);
                biome_texture_items_.push_back(tile);
            } else {
                QColor c = zones[o].color;
                c.setAlpha(90);
                img.setPixelColor(x, y, c);
            }
        }
    }

    biome_tint_item_->setPixmap(QPixmap::fromImage(img));
}
