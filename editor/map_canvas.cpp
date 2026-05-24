#include "map_canvas.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QGraphicsRectItem>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPen>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

#include "dialogs/forest_spawn_dialog.h"
#include "map/yaml_map_io.h"

#include "editor_constants.h"
#include "verificator.h"

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
    // añadir vista al layout
    layout->addWidget(view_);
    // botones guardar, zoom in y zoom out
    auto* save_button = new QPushButton(QStringLiteral("Guardar mapa"), this);
    auto* zoom_in_button = new QPushButton(QStringLiteral("Zoom in"), this);
    auto* zoom_out_button = new QPushButton(QStringLiteral("Zoom out"), this);
    // layout de botones
    auto* buttons = new QHBoxLayout();
    buttons->addWidget(save_button);
    buttons->addWidget(zoom_in_button);
    buttons->addWidget(zoom_out_button);
    layout->addLayout(buttons);
    // conectar botones a funciones
    connect(save_button, &QPushButton::clicked, this, &MapCanvas::saveMap);
    connect(zoom_in_button, &QPushButton::clicked, this, [this]() { zoomIn(); });
    connect(zoom_out_button, &QPushButton::clicked, this, [this]() { zoomOut(); });
}

void MapCanvas::createMap(const QString& map_id, const QString& map_name, int width, int height) {
    map_id_ = map_id;
    map_name_ = map_name;
    map_width_ = width;
    map_height_ = height;

    scene_->clear();
    controller_->reset();
    drawing_zone_ = false;
    clearZonePreview();
    // Ancho y alto de la escena
    const int scene_width = width * CELL_DISPLAY_SIZE;
    const int scene_height = height * CELL_DISPLAY_SIZE;

    auto* background = scene_->addRect(0, 0, scene_width, scene_height, QPen(Qt::darkGray),
                                       QBrush(QColor(235, 235, 220)));
    background->setZValue(-2);
    drawGrid();
    scene_->setSceneRect(0, 0, scene_width, scene_height);
    QTimer::singleShot(0, this, [this]() { applyInitialView(); });

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

    // escala de la vista para que quepa en la ventana
    const double fit_scale_x = (viewport_w * 0.92) / scene_width;
    const double fit_scale_y = (viewport_h * 0.92) / scene_height;
    const double fit_scale = std::min(fit_scale_x, fit_scale_y);

    // escala de la vista para que cada celda ocupe el tamaño TARGET_CELL_SCREEN_PX
    const double target_scale = static_cast<double>(TARGET_CELL_SCREEN_PX) / CELL_DISPLAY_SIZE;
    // escala de la vista para que quepa en la ventana y cada celda ocupe el tamaño
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
void MapCanvas::setActiveTool(const ToolInfo& tool) { active_tool_ = tool; }

QString MapCanvas::map_id() const { return map_id_; }

QString MapCanvas::map_name() const { return map_name_; }

int MapCanvas::map_width() const { return map_width_; }

int MapCanvas::map_height() const { return map_height_; }
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

    QString error;
    // poner respecitvo item
    switch (active_tool_.tool) {
        case EditorTool::PlayerSpawn:
            controller_->placePlayerSpawn(cell_x, cell_y, error);
            break;
        case EditorTool::Obstacle:
            if (!controller_->placeObstacle(active_tool_, cell_x, cell_y, error)) {
                QMessageBox::warning(this, QStringLiteral("Obstáculo"), error);
            }
            break;
        case EditorTool::CityZone:
            placeCityAt(cell_x, cell_y);
            break;
        case EditorTool::ForestZone:
            if (!drawing_zone_) {
                drawing_zone_ = true;
                zone_start_cell_ = QPoint(cell_x, cell_y);
                clearZonePreview();
            }
            break;
        default:
            break;
    }
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
    zone_preview_->setZValue(5);
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
    }
}
// finalizar dibujo de zona de bosque
void MapCanvas::finishForestZoneDraw(int end_cell_x, int end_cell_y) {
    drawing_zone_ = false;
    clearZonePreview();
    end_cell_x = std::clamp(end_cell_x, 0, map_width_ - 1);
    end_cell_y = std::clamp(end_cell_y, 0, map_height_ - 1);
    // normalizar rectángulo de la celda
    QRect rect = normalizedCellRect(zone_start_cell_, QPoint(end_cell_x, end_cell_y));
    // si rectángulo es de 1x1, buscar template de bosque
    if (rect.width() == 1 && rect.height() == 1) {
        if (const auto* forest =
                    templates_.find_forest(active_tool_.forest_template_id.toStdString())) {
            rect.setSize(QSize(forest->default_width, forest->default_height));
        }
    }

    const auto* forest = templates_.find_forest(active_tool_.forest_template_id.toStdString());
    if (!forest) {
        QMessageBox::warning(this, QStringLiteral("Bosque"),
                             QStringLiteral("Template de bosque inválido."));
        return;
    }
    // abrir diálogo de selección de criaturas
    ForestSpawnDialog dialog(*forest, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    // colocar zona de bosque
    QString error;
    const auto spawns = dialog.selected_spawns();
    if (!controller_->placeForestZone(active_tool_, rect.x(), rect.y(), rect.width(), rect.height(),
                                      spawns, error)) {
        QMessageBox::warning(this, QStringLiteral("Bosque"), error);
    }
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
    controller_->deleteAtCell(cell_x, cell_y);
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
    if (event->type() == QEvent::MouseMove && drawing_zone_) {
        auto* mouse_event = static_cast<QMouseEvent*>(event);
        handleMouseMove(mouse_event->pos());
        return true;
    }
    // suelto el click izquierdo
    if (event->type() == QEvent::MouseButtonRelease && drawing_zone_) {
        auto* mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() == Qt::LeftButton) {
            int cell_x = 0;
            int cell_y = 0;
            cellFromViewPos(mouse_event->pos(), cell_x, cell_y);
            finishForestZoneDraw(cell_x, cell_y);
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}
// guardar mapa
bool MapCanvas::saveMap() {
    const auto document = controller_->buildDocument(map_id_, map_name_, map_width_, map_height_);
    // validar documento
    Verificator verificator(document);
    QString error_title;
    QString error_message;
    if (!verificator.validate(error_title, error_message)) {
        QMessageBox::warning(this, error_title, error_message);
        return false;
    }
    // guardar documento
    const QString path = QStringLiteral("%1/%2.yaml").arg(SAVE_MAP, map_id_);
    if (!YamlMapIO::save(document, path.toStdString())) {
        QMessageBox::warning(this, QStringLiteral("Error"),
                             QStringLiteral("No se pudo guardar el YAML."));
        return false;
    }

    QMessageBox::information(this, QStringLiteral("Guardado"),
                             QStringLiteral("Mapa guardado en:\n%1").arg(path));
    return true;
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
