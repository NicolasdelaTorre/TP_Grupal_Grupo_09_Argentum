#include "forest_spawn_dialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

ForestSpawnDialog::ForestSpawnDialog(const ForestTemplate& forest_template, QWidget* parent):
        QDialog(parent) {
    setWindowTitle(QStringLiteral("Criaturas del bosque"));
    resize(420, 320);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
            QStringLiteral("Template: %1").arg(QString::fromStdString(forest_template.name))));

    auto* form = new QFormLayout();
    for (const auto& creature: forest_template.allowed_creatures) {
        auto* enabled = new QCheckBox(QStringLiteral("Habilitar"));
        auto* population = new QSpinBox();
        population->setRange(0, 999);
        population->setValue(10);
        population->setEnabled(false);

        connect(enabled, &QCheckBox::toggled, population, &QSpinBox::setEnabled);

        const QString label = QString::fromStdString(creature);
        form->addRow(label, enabled);
        form->addRow(QStringLiteral("%1 - cantidad").arg(label), population);

        entries_.push_back({enabled, population, creature});
    }

    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

std::vector<CreatureSpawn> ForestSpawnDialog::selected_spawns() const {
    std::vector<CreatureSpawn> spawns;
    for (const auto& entry: entries_) {
        if (!entry.enabled->isChecked()) {
            continue;
        }
        CreatureSpawn spawn;
        spawn.creature = entry.creature;
        spawn.max_population = entry.population->value();
        if (spawn.max_population > 0) {
            spawns.push_back(spawn);
        }
    }
    return spawns;
}
