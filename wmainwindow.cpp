/**
  * @file wmainwindow.cpp
  * Source file for the testing GUI main window.
  */

#include "wmainwindow.h"
#include "ui_wmainwindow.h"
#include "debugcheck.h"

WMainWindow::WMainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::WMainWindow)
{
    ui->setupUi(this);
    wiimote.start();
    connect(&wiimote, SIGNAL(updatedState()), this, SLOT(changeLabel()));
}

WMainWindow::~WMainWindow()
{
    delete ui;
}

void WMainWindow::changeLabel()
{
    //ui->label->setText(QString::number(this->wiimote.buttonData(), 16));
    //ui->button_up->setChecked(true);
    QWiimote::WiimoteButtons button_data = this->wiimote.buttonData();

    ui->radio_up->setChecked(false);
    ui->radio_left->setChecked(false);
    ui->radio_right->setChecked(false);
    ui->radio_down->setChecked(false);
    ui->radio_a->setChecked(false);
    ui->radio_b->setChecked(false);
    ui->radio_minus->setChecked(false);
    ui->radio_home->setChecked(false);
    ui->radio_plus->setChecked(false);
    ui->radio_1->setChecked(false);
    ui->radio_2->setChecked(false);

    if (button_data & QWiimote::ButtonUp)    ui->radio_up->setChecked(true);
    if (button_data & QWiimote::ButtonLeft)  ui->radio_left->setChecked(true);
    if (button_data & QWiimote::ButtonRight) ui->radio_right->setChecked(true);
    if (button_data & QWiimote::ButtonDown)  ui->radio_down->setChecked(true);
    if (button_data & QWiimote::ButtonA)     ui->radio_a->setChecked(true);
    if (button_data & QWiimote::ButtonB)     ui->radio_b->setChecked(true);
    if (button_data & QWiimote::ButtonMinus) ui->radio_minus->setChecked(true);
    if (button_data & QWiimote::ButtonHome)  ui->radio_home->setChecked(true);
    if (button_data & QWiimote::ButtonPlus)  ui->radio_plus->setChecked(true);
    if (button_data & QWiimote::ButtonOne)   ui->radio_1->setChecked(true);
    if (button_data & QWiimote::ButtonTwo)   ui->radio_2->setChecked(true);
}
