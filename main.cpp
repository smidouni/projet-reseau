#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(800, 600); // Taille de la fenêtre
    w.show();
    return a.exec();
}
