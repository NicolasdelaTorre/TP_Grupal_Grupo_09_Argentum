#ifndef ARGENTUM_EDITOR_DIALOGS_NEW_ENVIRONMENT_DIALOG_H
#define ARGENTUM_EDITOR_DIALOGS_NEW_ENVIRONMENT_DIALOG_H

#include <QDialog>
#include <QString>

#include "assets/templates/template_registry.h"

class QComboBox;
class QLineEdit;

class NewEnvironmentDialog: public QDialog {
    Q_OBJECT

public:
    NewEnvironmentDialog(const EntryTemplate& entry_template, const QString& suggested_name,
                         QWidget* parent = nullptr);

    QString environment_name() const;
    int environment_width() const;
    int environment_height() const;

private:
    QLineEdit* name_input_;
    QComboBox* size_preset_;
};

#endif
