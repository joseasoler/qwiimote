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

    test = new QIOWiimote(this);
    if (!test->open()) exit(1000);
    connect(this->test, SIGNAL(reportReady()), this, SLOT(changeLabel()));
}

void WMainWindow::changeLabel()
{
    QWiimoteReport report = test->getReport();
    this->ui->debug_label->setText(QString(report.data.toHex()));
    this->ui->time_label->setText(report.time.toString("ss:zzz"));

}

WMainWindow::~WMainWindow()
{
    delete ui;
}
