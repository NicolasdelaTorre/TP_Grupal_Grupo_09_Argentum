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
        error_title = QStringLiteral("Invalid map");
        error_message = QStringLiteral("The map must have width and height greater than zero.");
        return false;
    }
    if (document_.map.id.empty()) {
        error_title = QStringLiteral("Invalid map");
        error_message = QStringLiteral("The map must have an id.");
        return false;
    }
    return true;
}

bool Verificator::checkPlayerSpawn(QString& error_title, QString& error_message) const {
    if (!document_.player_spawn.placed) {
        error_title = QStringLiteral("Missing spawn");
        error_message = QStringLiteral("You must place exactly one player spawn.");
        return false;
    }
    return true;
}

bool Verificator::checkEntries(QString& error_title, QString& error_message) const {
    for (const auto& entry: document_.entries) {
        const auto environment_it =
                std::find_if(document_.environments.begin(), document_.environments.end(),
                             [&entry](const auto& env) { return env.id == entry.environment_id; });
        if (environment_it == document_.environments.end()) {
            error_title = QStringLiteral("Orphan entry");
            error_message = QStringLiteral("The entry '%1' points to a nonexistent environment '%2'.")
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
            error_title = QStringLiteral("Invalid environment");
            error_message = QStringLiteral("The environment '%1' has an invalid size.")
                                    .arg(QString::fromStdString(env.id));
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
            error_title = QStringLiteral("Environment without enclosure");
            error_message = QStringLiteral("The walls of environment '%1' do not enclose any area.")
                                    .arg(QString::fromStdString(env.id));
            return false;
        }

        if (env.player_spawn.placed) {
            const int sx = env.player_spawn.x;
            const int sy = env.player_spawn.y;
            const size_t idx = static_cast<size_t>(sy) * W + sx;
            if (is_wall[idx]) {
                error_title = QStringLiteral("Spawn on wall");
                error_message = QStringLiteral("The spawn of environment '%1' falls on a wall.")
                                        .arg(QString::fromStdString(env.id));
                return false;
            }
            if (is_exterior[idx]) {
                error_title = QStringLiteral("Spawn outside enclosure");
                error_message =
                        QStringLiteral(
                                "The spawn of environment '%1' must be inside the closed enclosure.")
                                .arg(QString::fromStdString(env.id));
                return false;
            }
        }
    }
    return true;
}
