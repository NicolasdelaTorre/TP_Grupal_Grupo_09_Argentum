#include "new_environment_dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

NewEnvironmentDialog::NewEnvironmentDialog(const QString& type_display_name,
                                           const QString& suggested_name, QWidget* parent):
        QDialog(parent) {
    setWindowTitle(QStringLiteral("New environment"));

    name_input_ = new QLineEdit(suggested_name);

    auto* type_field = new QLineEdit(type_display_name);
    type_field->setEnabled(false);

    auto* form = new QFormLayout();
    form->addRow(QStringLiteral("Type"), type_field);
    form->addRow(QStringLiteral("Name"), name_input_);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

QString NewEnvironmentDialog::environment_name() const { return name_input_->text().trimmed(); }
