#include "new_environment_dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

NewEnvironmentDialog::NewEnvironmentDialog(const QString& type_display_name,
                                           const QString& suggested_name, QWidget* parent):
        QDialog(parent) {
    setWindowTitle(QStringLiteral("New environment"));
    setMinimumSize(200, 180);

    name_input_ = new QLineEdit(suggested_name);
    name_input_->setMinimumWidth(160);

    auto* type_field = new QLineEdit(type_display_name);
    type_field->setEnabled(false);
    type_field->setMinimumWidth(160);

    auto* form = new QFormLayout();
    form->setContentsMargins(6, 6, 6, 0);
    form->addRow(QStringLiteral("Type"), type_field);
    form->addRow(QStringLiteral("Name"), name_input_);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->setCenterButtons(true);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

QString NewEnvironmentDialog::environment_name() const { return name_input_->text().trimmed(); }
