/*
 * This file is part of QWiimote.
 *
 * QWiimote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QWiimote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QWiimote. If not, see <http://www.gnu.org/licenses/>.
 */

/**
  * @file wmainwindow.cpp
  *
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
	ui->acceleration_x->setMinimum(-4000);
	ui->acceleration_x->setMaximum( 4000);
	ui->acceleration_x->setValue(0);
	ui->acceleration_y->setMinimum(-4000);
	ui->acceleration_y->setMaximum( 4000);
	ui->acceleration_y->setValue(0);
	ui->acceleration_z->setMinimum(-4000);
	ui->acceleration_z->setMaximum( 4000);
	ui->acceleration_z->setValue(0);

	wiimote.start(0);
	this->led_state = QWiimote::Led1;
	wiimote.setLeds(led_state);
	connect(&wiimote, SIGNAL(updatedButtons()), this, SLOT(changeButtons()));
	connect(&wiimote, SIGNAL(updatedAcceleration()), this, SLOT(changeAcceleration()));
	connect(&wiimote, SIGNAL(updatedBattery()), this, SLOT(changeBattery()));
	connect(&wiimote, SIGNAL(updatedOrientation()), this, SLOT(changeOrientation()));
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
	QVector3D acc = this->wiimote.acceleration();
	ui->acceleration_x->setValue(acc.x() * 1000);
	ui->acceleration_y->setValue(acc.y() * 1000);
	ui->acceleration_z->setValue(acc.z() * 1000);
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

	if (button_data & QWiimote::ButtonA)	  ui->radio_a->setChecked(true);
	if (button_data & QWiimote::ButtonB)	  ui->radio_b->setChecked(true);
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
	QMatrix4x4 orientation = this->wiimote.orientation();
	ui->glwidget->updateRotation(orientation);
}
