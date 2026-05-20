#ifndef ARGENTUM_EDITOR_DIALOGS_NEW_MAP_DIALOG_H
#define ARGENTUM_EDITOR_DIALOGS_NEW_MAP_DIALOG_H

#include <QDialog>
#include <QString>

class QComboBox;
class QLineEdit;

class NewMapDialog: public QDialog {
    Q_OBJECT

public:
    explicit NewMapDialog(QWidget* parent = nullptr);

    QString map_id() const;
    QString map_name() const;
    int map_width() const;
    int map_height() const;

private:
    QLineEdit* id_input_;
    QLineEdit* name_input_;
    QComboBox* size_preset_;
};

#endif
