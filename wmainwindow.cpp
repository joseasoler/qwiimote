/**
  * @file
  * Source file for the testing GUI main window.
  */

#include "wmainwindow.h"
#include "ui_wmainwindow.h"
#include "qiowiimote.h" // Should be removed once that QIOWiimote works.

WMainWindow::WMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::WMainWindow)
{
    ui->setupUi(this);

    QIOWiimote * test = new QIOWiimote(this);
    if (test->open()) exit(1000);
}

WMainWindow::~WMainWindow()
{
    delete ui;
}
