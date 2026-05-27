#include "new_environment_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSize>
#include <QVBoxLayout>

NewEnvironmentDialog::NewEnvironmentDialog(const EntryTemplate& entry_template,
                                           const QString& suggested_name, QWidget* parent):
        QDialog(parent) {
    setWindowTitle(QStringLiteral("Nuevo entorno"));

    name_input_ = new QLineEdit(suggested_name);

    size_preset_ = new QComboBox();
    for (const auto& option: entry_template.environment_sizes) {
        const QString label = QStringLiteral("%1 x %2").arg(option.width).arg(option.height);
        size_preset_->addItem(label, QSize(option.width, option.height));
    }

    auto* form = new QFormLayout();
    form->addRow(QStringLiteral("Tipo"), new QLineEdit(QString::fromStdString(entry_template.name)));
    form->itemAt(form->rowCount() - 1, QFormLayout::FieldRole)
            ->widget()
            ->setEnabled(false);
    form->addRow(QStringLiteral("Nombre"), name_input_);
    form->addRow(QStringLiteral("Tamaño"), size_preset_);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

QString NewEnvironmentDialog::environment_name() const { return name_input_->text().trimmed(); }

int NewEnvironmentDialog::environment_width() const {
    return size_preset_->currentData().toSize().width();
}

int NewEnvironmentDialog::environment_height() const {
    return size_preset_->currentData().toSize().height();
}
