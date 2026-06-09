#ifndef ARGENTUM_EDITOR_DIALOGS_NEW_ENVIRONMENT_DIALOG_H
#define ARGENTUM_EDITOR_DIALOGS_NEW_ENVIRONMENT_DIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;

class NewEnvironmentDialog: public QDialog {
    Q_OBJECT

public:
    NewEnvironmentDialog(const QString& type_display_name, const QString& suggested_name,
                         QWidget* parent = nullptr);

    QString environment_name() const;

private:
    QLineEdit* name_input_;
};

#endif
