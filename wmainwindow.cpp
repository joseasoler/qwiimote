/**
  * @file wmainwindow.cpp
  * Source file for the testing GUI main window.
  */

#include "wmainwindow.h"
#include "ui_wmainwindow.h"

WMainWindow::WMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::WMainWindow)
{
    ui->setupUi(this);
    wiimote.start();
}

WMainWindow::~WMainWindow()
{
    delete ui;
}
