/**
  * @file wmainwindow.cpp
  * Source file for the testing GUI main window.
  */

#include <cmath>
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

    wiimote.start(0);
    this->led_state = QWiimote::Led1;
    wiimote.setLeds(led_state);
    connect(&wiimote, SIGNAL(updatedButtons()), this, SLOT(changeButtons()));
    connect(&wiimote, SIGNAL(updatedAcceleration()), this, SLOT(changeAcceleration()));
    connect(&wiimote, SIGNAL(updatedBattery()), this, SLOT(changeBattery()));
    connect(&wiimote, SIGNAL(updatedMotionPlus()), this, SLOT(changeOrientation()));
    ui->report_acceleration->setChecked(false);
    ui->report_motionplus->setChecked(false);
    this->changeOrientation();
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
    if (checked) this->wiimote.setDataTypes(this->wiimote.dataTypes() | QWiimote::AccelerometerData);
    else this->wiimote.setDataTypes(this->wiimote.dataTypes() & ~QWiimote::AccelerometerData);
}

void WMainWindow::changeBattery()
{
    ui->battery_level->display(this->wiimote.batteryLevel());
}

void WMainWindow::on_report_motionplus_clicked(bool checked)
{
    if (checked) {
        this->wiimote.setDataTypes(this->wiimote.dataTypes() | QWiimote::MotionPlusData);
        ui->report_acceleration->setDisabled(true);
        ui->report_acceleration->setChecked(true);
    } else {
        this->wiimote.setDataTypes(this->wiimote.dataTypes() & ~QWiimote::MotionPlusData);
        ui->report_acceleration->setDisabled(false);
    }
}

void WMainWindow::changeOrientation()
{
    QQuaternion orientation = this->wiimote.orientation();
    qreal q0 = orientation.x();
    qreal q1 = orientation.y();
    qreal q2 = orientation.z();
    qreal q3 = orientation.scalar();
    qreal angle_1 = atan2(2 * (q0 * q1 + q2 * q3) , 1 - 2 * (q1 * q1 + q2 * q2));
    qreal angle_2 = asin(2 * (q0 * q2 - q3 * q1));
    qreal angle_3 = atan2(2 * (q0 * q3 + q1 * q2) , 1 - 2 * (q2 * q2 + q3 * q3));

    angle_1 = angle_1 * 180 / QW_PI;
    angle_2 = angle_2 * 180 / QW_PI;
    angle_3 = angle_3 * 180 / QW_PI;

    qDebug() << "Angles: " << angle_1 << " " << angle_2 << " " << angle_3;

    ui->angle_x->display(angle_1);
    ui->angle_y->display(angle_3);
    ui->angle_z->display(angle_2);
}
