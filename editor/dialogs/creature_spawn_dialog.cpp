#include "creature_spawn_dialog.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>

#include "editor_constants.h"

QString CreatureSpawnDialog::capitalize_first(const QString& text) {
    if (text.isEmpty()) {
        return text;
    }
    return text.left(1).toUpper() + text.mid(1);
}

int CreatureSpawnDialog::initial_population_for(const std::vector<CreatureSpawn>& initial_spawns,
                                                const std::string& creature) {
    const auto it = std::find_if(
            initial_spawns.begin(), initial_spawns.end(),
            [&creature](const CreatureSpawn& spawn) { return spawn.creature == creature; });
    return it == initial_spawns.end() ? CREATURE_SLIDER_DEFAULT : it->max_population;
}

CreatureSpawnDialog::CreatureSpawnDialog(const QString& window_title, const QString& display_title,
                                         const std::vector<std::string>& creatures,
                                         const std::vector<CreatureSpawn>& initial_spawns,
                                         QWidget* parent):
        QDialog(parent) {
    setWindowTitle(window_title);
    setMinimumSize(440, 360);

    auto* layout = new QVBoxLayout(this);

    auto* title = new QLabel(display_title);
    title->setAlignment(Qt::AlignCenter);
    QFont title_font = title->font();
    title_font.setPointSize(title_font.pointSize() + 4);
    title_font.setBold(true);
    title->setFont(title_font);
    layout->addWidget(title);
    layout->addSpacing(8);

    auto* grid = new QGridLayout();
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(2, 0);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(8);

    int row = 0;
    for (const auto& creature: creatures) {
        const QString creature_label = capitalize_first(QString::fromStdString(creature));

        auto* name_label = new QLabel(creature_label);
        auto* slider = new QSlider(Qt::Horizontal);
        slider->setRange(CREATURE_SLIDER_MIN, CREATURE_SLIDER_MAX);
        slider->setValue(initial_population_for(initial_spawns, creature));

        auto* value_label = new QLabel(QString::number(slider->value()));
        value_label->setMinimumWidth(CREATURE_VALUE_LABEL_WIDTH);
        value_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        connect(slider, &QSlider::valueChanged, value_label,
                [value_label](int value) { value_label->setText(QString::number(value)); });

        grid->addWidget(name_label, row, 0);
        grid->addWidget(slider, row, 1);
        grid->addWidget(value_label, row, 2);

        entries_.push_back({slider, creature});
        ++row;
    }

    layout->addLayout(grid, 1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

std::vector<CreatureSpawn> CreatureSpawnDialog::selected_spawns() const {
    std::vector<CreatureSpawn> spawns;
    for (const auto& entry: entries_) {
        const int value = entry.population->value();
        if (value <= 0) {
            continue;
        }
        CreatureSpawn spawn;
        spawn.creature = entry.creature;
        spawn.max_population = value;
        spawns.push_back(spawn);
    }
    return spawns;
}
