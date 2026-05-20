#include "new_map_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

NewMapDialog::NewMapDialog(QWidget* parent): QDialog(parent) {
    setWindowTitle(QStringLiteral("Nuevo mapa"));

    id_input_ = new QLineEdit(QStringLiteral("mapa_inicial"));
    name_input_ = new QLineEdit(QStringLiteral("Mapa Inicial"));

    size_preset_ = new QComboBox();
    size_preset_->addItem(QStringLiteral("100 x 100"), QSize(100, 100));
    size_preset_->addItem(QStringLiteral("250 x 250"), QSize(250, 250));
    size_preset_->addItem(QStringLiteral("500 x 500"), QSize(500, 500));

    auto* form = new QFormLayout();
    form->addRow(QStringLiteral("Id"), id_input_);
    form->addRow(QStringLiteral("Nombre"), name_input_);
    form->addRow(QStringLiteral("Tamaño"), size_preset_);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

QString NewMapDialog::map_id() const { return id_input_->text().trimmed(); }

QString NewMapDialog::map_name() const { return name_input_->text().trimmed(); }

int NewMapDialog::map_width() const { return size_preset_->currentData().toSize().width(); }

int NewMapDialog::map_height() const { return size_preset_->currentData().toSize().height(); }
