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
    ui->acceleration_x->setMinimum(0);
    ui->acceleration_x->setMaximum(0x3FF);
    ui->acceleration_x->setValue(0);
    ui->acceleration_y->setMinimum(0);
    ui->acceleration_y->setMaximum(0x3FF);
    ui->acceleration_y->setValue(0);
    ui->acceleration_z->setMinimum(0);
    ui->acceleration_z->setMaximum(0x3FF);
    ui->acceleration_z->setValue(0);

    ui->real_acceleration_x->setSmallDecimalPoint(true);
    ui->real_acceleration_y->setSmallDecimalPoint(true);
    ui->real_acceleration_z->setSmallDecimalPoint(true);

    ui->real_acceleration_x->setNumDigits(10);
    ui->real_acceleration_y->setNumDigits(10);
    ui->real_acceleration_z->setNumDigits(10);

    ui->real_acceleration_x->setSegmentStyle(QLCDNumber::Flat);
    ui->real_acceleration_y->setSegmentStyle(QLCDNumber::Flat);
    ui->real_acceleration_z->setSegmentStyle(QLCDNumber::Flat);

    ui->battery_level->setNumDigits(3);
    ui->battery_level->setSegmentStyle(QLCDNumber::Flat);

    count = 0;

    wiimote.start(0);
    this->led_state = QWiimote::Led1;
    wiimote.setLeds(led_state);
    connect(&wiimote, SIGNAL(updatedButtons()), this, SLOT(changeButtons()));
    connect(&wiimote, SIGNAL(updatedAcceleration()), this, SLOT(changeAcceleration()));
    connect(&wiimote, SIGNAL(updatedBattery()), this, SLOT(changeBattery()));
    ui->report_acceleration->setChecked(false);
}

WMainWindow::~WMainWindow()
{
    delete ui;
}

void WMainWindow::changeAcceleration()
{
    ui->acceleration_x->setValue(this->wiimote.rawAccelerationX());
    ui->acceleration_y->setValue(this->wiimote.rawAccelerationY());
    ui->acceleration_z->setValue(this->wiimote.rawAccelerationZ());

    count++;
    if (count % 500) {
        ui->real_acceleration_x->display(this->wiimote.accelerationX());
        ui->real_acceleration_y->display(this->wiimote.accelerationY());
        ui->real_acceleration_z->display(this->wiimote.accelerationZ());
    }
}

void WMainWindow::changeButtons()
{
    QWiimote::WiimoteButtons button_data = this->wiimote.buttonData();
    bool leds_changed = false;

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

    if (button_data & QWiimote::ButtonUp) {
        ui->radio_up->setChecked(true);
        this->led_state ^= QWiimote::Led1;
        leds_changed = true;
    }

    if (button_data & QWiimote::ButtonRight) {
        ui->radio_right->setChecked(true);
        this->led_state ^= QWiimote::Led2;
        leds_changed = true;
    }

    if (button_data & QWiimote::ButtonDown) {
        ui->radio_down->setChecked(true);
        this->led_state ^= QWiimote::Led3;
        leds_changed = true;
    }

    if (button_data & QWiimote::ButtonLeft) {
        ui->radio_left->setChecked(true);
        this->led_state ^= QWiimote::Led4;
        leds_changed = true;
    }

    if (button_data & QWiimote::ButtonA) {
        ui->radio_a->setChecked(true);
        this->led_state ^= QWiimote::Rumble;
        leds_changed = true;
    }

    if (button_data & QWiimote::ButtonB)     ui->radio_b->setChecked(true);
    if (button_data & QWiimote::ButtonMinus) ui->radio_minus->setChecked(true);
    if (button_data & QWiimote::ButtonHome)  ui->radio_home->setChecked(true);
    if (button_data & QWiimote::ButtonPlus)  ui->radio_plus->setChecked(true);
    if (button_data & QWiimote::ButtonOne)   ui->radio_1->setChecked(true);
    if (button_data & QWiimote::ButtonTwo)   ui->radio_2->setChecked(true);

    if (leds_changed) wiimote.setLeds(led_state);
}

void WMainWindow::on_report_acceleration_clicked(bool checked)
{
    if (checked) this->wiimote.setDataTypes(QWiimote::AccelerometerData);
    else this->wiimote.setDataTypes(0);
}

void WMainWindow::changeBattery()
{
    ui->battery_level->display(this->wiimote.batteryLevel());
}
