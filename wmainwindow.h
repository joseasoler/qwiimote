/**
  * @file
  * Header file for the testing GUI main window.
  */

#ifndef WMAINWINDOW_H
#define WMAINWINDOW_H

#include <QtGui/QMainWindow>
#include "qiowiimote.h" // For testing QIOWiimote only, remove later.


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
public slots:           // For testing QIOWiimote only, remove later.
    void changeLabel(); // For testing QIOWiimote only, remove later.

private:
    Ui::WMainWindow *ui;
    QIOWiimote * test;
};

#endif // WMAINWINDOW_H
