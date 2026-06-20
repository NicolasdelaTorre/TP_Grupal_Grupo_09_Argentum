#ifndef ARGENTUM_EDITOR_DIALOGS_CREATURE_SPAWN_DIALOG_H
#define ARGENTUM_EDITOR_DIALOGS_CREATURE_SPAWN_DIALOG_H

#include <QDialog>
#include <QString>
#include <string>
#include <vector>

#include "map/map_data.h"

class QSlider;

class CreatureSpawnDialog: public QDialog {
    Q_OBJECT

public:
    CreatureSpawnDialog(const QString& window_title, const QString& display_title,
                        const std::vector<std::string>& creatures,
                        const std::vector<CreatureSpawn>& initial_spawns = {},
                        QWidget* parent = nullptr);

    std::vector<CreatureSpawn> selected_spawns() const;

private:
    struct CreatureEntry {
        QSlider* population = nullptr;
        std::string creature;
    };

    static QString capitalize_first(const QString& text);
    static int initial_population_for(const std::vector<CreatureSpawn>& initial_spawns,
                                      const std::string& creature);

    std::vector<CreatureEntry> entries_;
};

#endif
