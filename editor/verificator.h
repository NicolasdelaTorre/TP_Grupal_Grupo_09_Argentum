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

    bool checkMap(QString& error_title, QString& error_message) const;
    bool checkPlayerSpawn(QString& error_title, QString& error_message) const;
    bool checkEntries(QString& error_title, QString& error_message) const;
    bool checkEnvironments(QString& error_title, QString& error_message) const;
};

#endif
