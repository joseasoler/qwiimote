/**
  * @file main.cpp
  * Main source file.
  */

#include <QtGui/QApplication>
#include "wmainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WMainWindow w;
    w.show();
    return a.exec();
}
