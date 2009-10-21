/**
  * @file
  * Source file for the testing GUI main window.
  */

#include "wmainwindow.h"
#include "ui_wmainwindow.h"

WMainWindow::WMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::WMainWindow)
{
    ui->setupUi(this);
    // Ugly way of testing, I know
    QWiimote aux;
    if (aux.findWiimote()) exit(6396);
}

WMainWindow::~WMainWindow()
{
    delete ui;
}
