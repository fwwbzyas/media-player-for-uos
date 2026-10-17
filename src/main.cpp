#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QtMultimedia/QtMultimedia>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/icon.png"));
    MainWindow w;
    w.show();

    return a.exec();
}
