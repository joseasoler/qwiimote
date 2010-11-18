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
 * @file wmainwindow.h
 *
 * Header file for the testing GUI main window.
 */

#ifndef WMAINWINDOW_H
#define WMAINWINDOW_H

#include <QtGui/QMainWindow>
#include "qwiimote/qwiimote.h"

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
	void changeButtons();
	void changeAcceleration();
	void changeBattery();
	void changeOrientation();

private:
	Ui::WMainWindow *ui;
	QWiimote wiimote;
	QWiimote::WiimoteLeds led_state;

private slots:
	void on_report_motionplus_clicked(bool checked);
	void on_report_acceleration_clicked(bool checked);
};

#endif // WMAINWINDOW_H
