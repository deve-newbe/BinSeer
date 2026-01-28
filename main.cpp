#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    QIcon appIcon(":/icon/logo.png"); // path to your logo file
    a.setWindowIcon(appIcon);

    QCoreApplication::setApplicationName(PROJECT_NAME);
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    MainWindow w;
    w.show();
    return a.exec();
}
