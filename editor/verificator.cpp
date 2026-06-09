#include "verificator.h"

#include <algorithm>
#include <vector>

#include "map/biome_grid.h"

#include "editor_constants.h"

bool Verificator::validate(QString& error_title, QString& error_message) const {
    if (!checkMap(error_title, error_message)) {
        return false;
    }
    if (!checkPlayerSpawn(error_title, error_message)) {
        return false;
    }
    if (!checkObstacles(error_title, error_message)) {
        return false;
    }
    if (!checkZones(error_title, error_message)) {
        return false;
    }
    if (!checkEntries(error_title, error_message)) {
        return false;
    }
    if (!checkEnvironments(error_title, error_message)) {
        return false;
    }
    return true;
}

Verificator::Verificator(const MapDocument& document): document_(document) {}

bool Verificator::checkMap(QString& error_title, QString& error_message) const {
    if (document_.map.width <= 0 || document_.map.height <= 0) {
        error_title = QStringLiteral("Mapa inválido");
        error_message = QStringLiteral("El mapa debe tener ancho y alto mayores a cero.");
        return false;
    }
    if (document_.map.id.empty()) {
        error_title = QStringLiteral("Mapa inválido");
        error_message = QStringLiteral("El mapa debe tener un id.");
        return false;
    }
    return true;
}

bool Verificator::checkPlayerSpawn(QString& error_title, QString& error_message) const {
    if (!document_.player_spawn.placed) {
        error_title = QStringLiteral("Spawn faltante");
        error_message = QStringLiteral("Debés colocar exactamente un spawn de jugador.");
        return false;
    }

    const auto& spawn = document_.player_spawn;
    if (spawn.x < 0 || spawn.y < 0 || spawn.x >= document_.map.width ||
        spawn.y >= document_.map.height) {
        error_title = QStringLiteral("Spawn inválido");
        error_message = QStringLiteral("El spawn del jugador está fuera del mapa.");
        return false;
    }
    return true;
}

bool Verificator::checkObstacles(QString& error_title, QString& error_message) const {
    const auto it = std::find_if(document_.obstacles.begin(), document_.obstacles.end(),
                                 [this](const auto& obstacle) {
                                     return obstacle.x < 0 || obstacle.y < 0 ||
                                            obstacle.x + obstacle.width > document_.map.width ||
                                            obstacle.y + obstacle.height > document_.map.height;
                                 });
    if (it != document_.obstacles.end()) {
        error_title = QStringLiteral("Obstáculo inválido");
        error_message = QStringLiteral("El obstáculo '%1' está fuera de los límites del mapa.")
                                .arg(QString::fromStdString(it->id));
        return false;
    }
    return true;
}

bool Verificator::checkZones(QString& error_title, QString& error_message) const {
    const auto it =
            std::find_if(document_.zones.begin(), document_.zones.end(), [this](const auto& zone) {
                return zone.area_x < 0 || zone.area_y < 0 ||
                       zone.area_x + zone.area_width > document_.map.width ||
                       zone.area_y + zone.area_height > document_.map.height;
            });
    if (it != document_.zones.end()) {
        error_title = QStringLiteral("Zona inválida");
        error_message = QStringLiteral("La zona '%1' está fuera de los límites del mapa.")
                                .arg(QString::fromStdString(it->id));
        return false;
    }
    return true;
}

bool Verificator::checkEntries(QString& error_title, QString& error_message) const {
    for (const auto& entry: document_.entries) {
        if (entry.x < 0 || entry.y < 0 || entry.x + entry.width > document_.map.width ||
            entry.y + entry.height > document_.map.height) {
            error_title = QStringLiteral("Entrada inválida");
            error_message = QStringLiteral("La entrada '%1' está fuera de los límites del mapa.")
                                    .arg(QString::fromStdString(entry.id));
            return false;
        }

        const auto environment_it =
                std::find_if(document_.environments.begin(), document_.environments.end(),
                             [&entry](const auto& env) { return env.id == entry.environment_id; });
        if (environment_it == document_.environments.end()) {
            error_title = QStringLiteral("Entrada huérfana");
            error_message = QStringLiteral("La entrada '%1' apunta a un entorno inexistente '%2'.")
                                    .arg(QString::fromStdString(entry.id),
                                         QString::fromStdString(entry.environment_id));
            return false;
        }
    }
    return true;
}

bool Verificator::checkEnvironments(QString& error_title, QString& error_message) const {
    for (const auto& env: document_.environments) {
        if (env.width <= 0 || env.height <= 0) {
            error_title = QStringLiteral("Entorno inválido");
            error_message = QStringLiteral("El entorno '%1' tiene tamaño inválido.")
                                    .arg(QString::fromStdString(env.id));
            return false;
        }
        const auto invalid_obstacle = std::find_if(
                env.obstacles.begin(), env.obstacles.end(), [&env](const auto& obstacle) {
                    return obstacle.x < 0 || obstacle.y < 0 ||
                           obstacle.x + obstacle.width > env.width ||
                           obstacle.y + obstacle.height > env.height;
                });
        if (invalid_obstacle != env.obstacles.end()) {
            error_title = QStringLiteral("Obstáculo inválido");
            error_message =
                    QStringLiteral("El obstáculo '%1' del entorno '%2' está fuera de los límites.")
                            .arg(QString::fromStdString(invalid_obstacle->id),
                                 QString::fromStdString(env.id));
            return false;
        }

        const auto invalid_wall =
                std::find_if(env.walls.begin(), env.walls.end(), [&env](const auto& wall) {
                    return wall.x < 0 || wall.y < 0 || wall.x + wall.width > env.width ||
                           wall.y + wall.height > env.height;
                });
        if (invalid_wall != env.walls.end()) {
            error_title = QStringLiteral("Pared inválida");
            error_message =
                    QStringLiteral("La pared '%1' del entorno '%2' está fuera de los límites.")
                            .arg(QString::fromStdString(invalid_wall->id),
                                 QString::fromStdString(env.id));
            return false;
        }

        const auto invalid_exit =
                std::find_if(env.exits.begin(), env.exits.end(), [&env](const auto& exit) {
                    return exit.x < 0 || exit.y < 0 || exit.x + exit.width > env.width ||
                           exit.y + exit.height > env.height;
                });
        if (invalid_exit != env.exits.end()) {
            error_title = QStringLiteral("Salida inválida");
            error_message =
                    QStringLiteral("La salida '%1' del entorno '%2' está fuera de los límites.")
                            .arg(QString::fromStdString(invalid_exit->id),
                                 QString::fromStdString(env.id));
            return false;
        }

        if (env.walls.empty()) {
            continue;
        }

        const int W = env.width;
        const int H = env.height;
        std::vector<bool> is_wall(static_cast<size_t>(W) * H, false);
        for (const auto& wall: env.walls) {
            for (int dy = 0; dy < wall.height; ++dy) {
                for (int dx = 0; dx < wall.width; ++dx) {
                    const int cx = wall.x + dx;
                    const int cy = wall.y + dy;
                    if (cx >= 0 && cy >= 0 && cx < W && cy < H) {
                        is_wall[static_cast<size_t>(cy) * W + cx] = true;
                    }
                }
            }
        }

        const std::vector<bool> is_exterior = computeExteriorCells(W, H, is_wall);

        bool has_interior = false;
        for (size_t i = 0; i < is_wall.size(); ++i) {
            if (!is_wall[i] && !is_exterior[i]) {
                has_interior = true;
                break;
            }
        }
        if (!has_interior) {
            error_title = QStringLiteral("Entorno sin recinto");
            error_message = QStringLiteral("Las paredes del entorno '%1' no encierran ningún área.")
                                    .arg(QString::fromStdString(env.id));
            return false;
        }

        if (env.player_spawn.placed) {
            const int sx = env.player_spawn.x;
            const int sy = env.player_spawn.y;
            if (sx < 0 || sy < 0 || sx >= W || sy >= H) {
                error_title = QStringLiteral("Spawn inválido");
                error_message = QStringLiteral("El spawn del entorno '%1' está fuera del entorno.")
                                        .arg(QString::fromStdString(env.id));
                return false;
            }
            const size_t idx = static_cast<size_t>(sy) * W + sx;
            if (is_wall[idx]) {
                error_title = QStringLiteral("Spawn sobre pared");
                error_message = QStringLiteral("El spawn del entorno '%1' cae sobre una pared.")
                                        .arg(QString::fromStdString(env.id));
                return false;
            }
            if (is_exterior[idx]) {
                error_title = QStringLiteral("Spawn fuera del recinto");
                error_message =
                        QStringLiteral(
                                "El spawn del entorno '%1' debe estar dentro del recinto cerrado.")
                                .arg(QString::fromStdString(env.id));
                return false;
            }
        }
    }
    return true;
}
