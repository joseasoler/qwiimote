/**
  * @file wmainwindow.h
  * Header file for the testing GUI main window.
  */

#ifndef WMAINWINDOW_H
#define WMAINWINDOW_H

#include <QtGui/QMainWindow>
#include "qwiimote.h"

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

public slots:
    void changeLabel();

private:
    Ui::WMainWindow *ui;
    QWiimote wiimote;
    QWiimote::WiimoteLeds led_state;

private slots:
    void on_report_acceleration_clicked(bool checked);
};

#endif // WMAINWINDOW_H
