#include "ui/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    srand(static_cast<unsigned>(time(nullptr)));
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
