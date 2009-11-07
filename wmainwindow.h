/**
  * @file wmainwindow.h
  * Header file for the testing GUI main window.
  */

#ifndef WMAINWINDOW_H
#define WMAINWINDOW_H

#include <QtGui/QMainWindow>

namespace Ui
{
    class WMainWindow;
}

class WMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    WMainWindow(QWidget *parent = 0);
    ~WMainWindow();

private:
    Ui::WMainWindow *ui;
};

#endif // WMAINWINDOW_H
