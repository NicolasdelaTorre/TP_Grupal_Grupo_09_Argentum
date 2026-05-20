#ifndef ARGENTUM_EDITOR_VERIFICATOR_H
#define ARGENTUM_EDITOR_VERIFICATOR_H

#include <QString>

#include "map/map_data.h"

class Verificator {
public:
    explicit Verificator(const MapDocument& document);

    bool validate(QString& error_title, QString& error_message) const;

private:
    const MapDocument& document_;

    bool check_map(QString& error_title, QString& error_message) const;
    bool check_player_spawn(QString& error_title, QString& error_message) const;
    bool check_obstacles(QString& error_title, QString& error_message) const;
    bool check_zones(QString& error_title, QString& error_message) const;
};

#endif
