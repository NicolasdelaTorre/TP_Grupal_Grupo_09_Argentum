#include "verificator.h"

#include "editor_constants.h"

bool Verificator::validate(QString& error_title, QString& error_message) const {
    if (!check_map(error_title, error_message)) {
        return false;
    }
    if (!check_player_spawn(error_title, error_message)) {
        return false;
    }
    if (!check_obstacles(error_title, error_message)) {
        return false;
    }
    if (!check_zones(error_title, error_message)) {
        return false;
    }
    return true;
}

Verificator::Verificator(const MapDocument& document): document_(document) {}

bool Verificator::check_map(QString& error_title, QString& error_message) const {
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

bool Verificator::check_player_spawn(QString& error_title, QString& error_message) const {
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

bool Verificator::check_obstacles(QString& error_title, QString& error_message) const {
    for (const auto& obstacle: document_.obstacles) {
        if (obstacle.x < 0 || obstacle.y < 0 ||
            obstacle.x + obstacle.width > document_.map.width ||
            obstacle.y + obstacle.height > document_.map.height) {
            error_title = QStringLiteral("Obstáculo inválido");
            error_message =
                    QStringLiteral("El obstáculo '%1' está fuera de los límites del mapa.")
                            .arg(QString::fromStdString(obstacle.id));
            return false;
        }
    }
    return true;
}

bool Verificator::check_zones(QString& error_title, QString& error_message) const {
    for (const auto& zone: document_.zones) {
        if (zone.area_x < 0 || zone.area_y < 0 ||
            zone.area_x + zone.area_width > document_.map.width ||
            zone.area_y + zone.area_height > document_.map.height) {
            error_title = QStringLiteral("Zona inválida");
            error_message = QStringLiteral("La zona '%1' está fuera de los límites del mapa.")
                                     .arg(QString::fromStdString(zone.id));
            return false;
        }
    }
    return true;
}
