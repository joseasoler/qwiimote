#include "wmainwindow.h"
#include "ui_wmainwindow.h"

WMainWindow::WMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::WMainWindow)
{
    ui->setupUi(this);
}

WMainWindow::~WMainWindow()
{
    delete ui;
}
