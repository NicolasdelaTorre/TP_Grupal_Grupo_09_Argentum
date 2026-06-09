#ifndef ARGENTUM_EDITOR_DIALOGS_NEW_ENVIRONMENT_DIALOG_H
#define ARGENTUM_EDITOR_DIALOGS_NEW_ENVIRONMENT_DIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;

class NewEnvironmentDialog: public QDialog {
    Q_OBJECT

public:
    // `type_display_name` es el nombre del tipo de entorno (Cueva/Mazmorra), que
    // queda fijo según el template de la entry y se muestra como solo lectura.
    NewEnvironmentDialog(const QString& type_display_name, const QString& suggested_name,
                         QWidget* parent = nullptr);

    QString environment_name() const;

private:
    QLineEdit* name_input_;
};

#endif
