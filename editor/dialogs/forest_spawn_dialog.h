#ifndef ARGENTUM_EDITOR_DIALOGS_FOREST_SPAWN_DIALOG_H
#define ARGENTUM_EDITOR_DIALOGS_FOREST_SPAWN_DIALOG_H

#include <QDialog>
#include <vector>

#include "map/map_data.h"
#include "assets/templates/template_registry.h"

class QCheckBox;
class QSpinBox;

class ForestSpawnDialog: public QDialog {
    Q_OBJECT

public:
    ForestSpawnDialog(const ForestTemplate& forest_template, QWidget* parent = nullptr);

    std::vector<CreatureSpawn> selected_spawns() const;

private:
    struct CreatureEntry {
        QCheckBox* enabled = nullptr;
        QSpinBox* population = nullptr;
        std::string creature;
    };

    std::vector<CreatureEntry> entries_;
};

#endif
