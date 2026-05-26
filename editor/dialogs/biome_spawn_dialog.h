#ifndef ARGENTUM_EDITOR_DIALOGS_BIOME_SPAWN_DIALOG_H
#define ARGENTUM_EDITOR_DIALOGS_BIOME_SPAWN_DIALOG_H

#include <QDialog>
#include <vector>

#include "map/map_data.h"
#include "assets/templates/template_registry.h"

class QSlider;

class BiomeSpawnDialog: public QDialog {
    Q_OBJECT

public:
    BiomeSpawnDialog(const BiomeTemplate& biome_template, QWidget* parent = nullptr);

    std::vector<CreatureSpawn> selected_spawns() const;

private:
    struct CreatureEntry {
        QSlider* population = nullptr;
        std::string creature;
    };

    std::vector<CreatureEntry> entries_;
};

#endif
