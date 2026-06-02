#include <QApplication>
#include <QFile>

#include "editor_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QFile qss(QStringLiteral(":/styles/editor.qss"));
    if (qss.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QString::fromUtf8(qss.readAll()));
        qss.close();
    }

    EditorWindow window;
    window.show();
    return app.exec();
}
