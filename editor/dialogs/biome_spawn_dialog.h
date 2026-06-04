#ifndef ARGENTUM_EDITOR_DIALOGS_BIOME_SPAWN_DIALOG_H
#define ARGENTUM_EDITOR_DIALOGS_BIOME_SPAWN_DIALOG_H

#include <QDialog>
#include <string>
#include <vector>

#include "assets/templates/template_registry.h"
#include "map/map_data.h"

class QSlider;

class BiomeSpawnDialog: public QDialog {
    Q_OBJECT

public:
    explicit BiomeSpawnDialog(const BiomeTemplate& biome_template, QWidget* parent = nullptr);
    BiomeSpawnDialog(const BiomeTemplate& biome_template,
                     const std::vector<CreatureSpawn>& initial_spawns, QWidget* parent = nullptr);

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
