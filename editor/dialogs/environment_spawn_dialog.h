#ifndef ARGENTUM_EDITOR_DIALOGS_ENVIRONMENT_SPAWN_DIALOG_H
#define ARGENTUM_EDITOR_DIALOGS_ENVIRONMENT_SPAWN_DIALOG_H

#include <QDialog>
#include <QString>
#include <string>
#include <vector>

#include "map/map_data.h"

class QSlider;

class EnvironmentSpawnDialog: public QDialog {
    Q_OBJECT

public:
    EnvironmentSpawnDialog(const QString& environment_name,
                           const std::vector<std::string>& creatures, QWidget* parent = nullptr);
    EnvironmentSpawnDialog(const QString& environment_name,
                           const std::vector<std::string>& creatures,
                           const std::vector<CreatureSpawn>& initial_spawns,
                           QWidget* parent = nullptr);

    std::vector<CreatureSpawn> selected_spawns() const;

private:
    struct CreatureEntry {
        QSlider* population = nullptr;
        std::string creature;
    };

    static int initial_population_for(const std::vector<CreatureSpawn>& initial_spawns,
                                      const std::string& creature);

    std::vector<CreatureEntry> entries_;
};

#endif
