/**
  * @file qwiimote.cpp
  * Source file for the QWiimote class.
  */

#include <cmath>
#include "qwiimote.h"
#include "debugcheck.h"

const quint16 QWiimote::MOTIONPLUS_TIME = 8000;
const qreal   QWiimote::DEGREES_PER_SECOND_SLOW = 8192.0 / 594.0;
const qreal   QWiimote::DEGREES_PER_SECOND_FAST = QWiimote::DEGREES_PER_SECOND_SLOW / 2000 / 440;
const quint8  QWiimote::MOTIONPLUS_THRESHOLD = 30;

/**
  * Creates a new QWiimote instance.
  * @param parent The parent of this instance.
  */
QWiimote::QWiimote(QObject * parent) : QObject(parent), io_wiimote(this)
{
	this->motionplus_orientation = QQuaternion::fromAxisAndAngle(0, 0, 1, 0);
}

/**
  * Destructor of QWiimote.
  * @todo Check if it should call to close.
  */
QWiimote::~QWiimote()
{
}

/**
  * The QWiimote starts working.
  * @todo This is just a preliminary version of start.
  * @return true if the QWiimote started correctly.
  */
bool QWiimote::start(QWiimote::DataTypes new_data_types)
{
	if (this->io_wiimote.open()) {
		this->setDataTypes(new_data_types);

		// All reports are ignored until the calibration data is received.
		connect(&io_wiimote, SIGNAL(reportReady(QWiimoteReport)), this, SLOT(getCalibrationReport(QWiimoteReport)));
		this->requestCalibrationData();
		// Initialize internal values.
		data_types = 0;
		this->motionplus_state = QWiimote::MotionPlusInactive;
		this->status_requested = false;
		this->battery_level = 0;
		this->battery_empty = false;
		this->motionplus_polling = NULL;

		return true;
	}

	return false;
}

/**
  * The QWiimote stops working and disconnects.
  * @todo This is just a preliminary version of stop.
  */
void QWiimote::stop()
{
	this->io_wiimote.close();
}

/**
  * Check what data types are currently being reported.
  * @return Flags of the current data types.
  */
QWiimote::DataTypes QWiimote::dataTypes() const
{
	return this->data_types;
}

/**
  * Sets what data types will be reported.
  * @param new_data_types Flags of the data types to report.
  */
void QWiimote::setDataTypes(QWiimote::DataTypes new_data_types)
{
	if (new_data_types & QWiimote::MotionPlusData && this->motionplus_state == QWiimote::MotionPlusInactive) {
		new_data_types |= QWiimote::AccelerometerData; // MotionPlus always activates AccelerometerData.
		this->motionplus_state = QWiimote::MotionPlusActivated;

		if (this->motionplus_polling == NULL) {// Start MotionPlus polling.
			motionplus_polling = new QTimer(this);
			connect(motionplus_polling, SIGNAL(timeout()), this, SLOT(pollMotionPlus()));
			motionplus_polling->start(1000);
			this->pollMotionPlus();
		}
	} else if (!(new_data_types & QWiimote::MotionPlusData) &&
			   (this->motionplus_state == QWiimote::MotionPlusWorking ||
				this->motionplus_state == QWiimote::MotionPlusCalibrated)) {
		this->motionplus_state = QWiimote::MotionPlusInactive;
		if (this->motionplus_polling != NULL) {// Stop MotionPlus polling.
			this->disableMotionPlus();
			disconnect(motionplus_polling, SIGNAL(timeout()), this, SLOT(pollMotionPlus()));
			delete motionplus_polling;
			motionplus_polling = NULL;
		}
	}
	this->data_types = new_data_types;
	qDebug() << "Data types: " << this->data_types;
	qDebug() << "MotionPlus state: " << this->motionplus_state;

	send_buffer[0] = 0x12;

	if ((this->data_types & QWiimote::MotionPlusData) != 0 &&
		(this->motionplus_state == QWiimote::MotionPlusWorking ||
		 this->motionplus_state == QWiimote::MotionPlusCalibrated)) {
		send_buffer[1] = 0x04; // Continuous reporting required.
		send_buffer[2] = 0x35;
	} else if ((this->data_types & QWiimote::AccelerometerData) != 0 && this->motionplus_state != QWiimote::MotionPlusWorking) {
		send_buffer[1] = 0x04; // Continuous reporting required.
		send_buffer[2] = 0x31;
	}
	else {
		send_buffer[1] = 0x00; // Continuous reporting not required.
		send_buffer[2] = 0x30;
		resetAccelerationData();
	}

	send_buffer[1] |= this->led_data & QWiimote::Rumble;

	this->io_wiimote.writeReport(send_buffer, 3);
}

/**
  * Sets what leds will be turned on.
  * @param leds Flags of the leds to turn on.
  */
void QWiimote::setLeds(QWiimote::WiimoteLeds leds)
{
	this->led_data = leds;
	send_buffer[0] = 0x11; // LED report.
	send_buffer[1] = leds; // LED status.
	this->io_wiimote.writeReport(send_buffer, 2);
}

/**
  * Check what buttons are pressed now.
  * @return Flags of the currently pressed buttons.
  */
QWiimote::WiimoteButtons QWiimote::buttonData() const
{
	return this->button_data;
}

/**
  * Allows to know what leds are on right now.
  * @return Flags of the used leds.
  */
QWiimote::WiimoteLeds QWiimote::leds() const
{
	return this->led_data;
}

/**
  * @return Raw acceleration on the X axis.
  */
quint16 QWiimote::rawAccelerationX() const
{
	return this->x_acceleration;
}

/**
  * @return Raw acceleration on the Y axis.
  */
quint16 QWiimote::rawAccelerationY() const
{
	return this->y_acceleration;
}

/**
  * @return Raw acceleration on the Z axis.
  */
quint16 QWiimote::rawAccelerationZ() const
{
	return this->z_acceleration;
}

/**
  * @return Calibrated acceleration on the X axis.
  */
qreal QWiimote::accelerationX() const
{
	return this->x_calibrated_acceleration;
}

/**
  * @return Calibrated acceleration on the Y axis.
  */
qreal QWiimote::accelerationY() const
{
	return this->y_calibrated_acceleration;
}

/**
  * @return Calibrated acceleration on the Z axis.
  */
qreal QWiimote::accelerationZ() const
{
	return this->y_calibrated_acceleration;
}

/**
  * Get the current battery level of this Wiimote.
  * @return Battery level of the Wiimote.
  */
quint8 QWiimote::batteryLevel() const
{
	return this->battery_level;
}

/**
  * Allows to know if the battery is empty or not.
  * @return True if the battery is empty, false otherwise.
  */
bool QWiimote::batteryEmpty() const
{
	return this->battery_empty;
}


/**
  * Request the calibration data from the Wiimote.
  * @return True if the report was sent correctly.
  */
bool QWiimote::requestCalibrationData()
{
	send_buffer[0] = 0x17; // Report type.
	send_buffer[1] = 0x00 | (this->led_data & QWiimote::Rumble); // Read from the EEPROM.
	send_buffer[2] = 0x00; // Memory position.
	send_buffer[3] = 0x00;
	send_buffer[4] = 0x16;
	send_buffer[5] = 0x00; // Data size.
	send_buffer[6] = 0x08;

	return this->io_wiimote.writeReport(send_buffer, 7);
}

/**
  * This function processes all incoming reports until a report with the acceleration data is received.
  * @param report Received report.
  */
void QWiimote::getCalibrationReport(QWiimoteReport report)
{
	qDebug() << "Receiving the report " << report.data.toHex() << " from the wiimote.";
	if (report.data[0] == (char)0x21) {
		// Get the required calibration values from the report.
		this->x_zero_acceleration = ((report.data[6] & 0xFF) << 2) + ((report.data[9] & 0x30) >> 4);
		this->y_zero_acceleration = ((report.data[7] & 0xFF) << 2) + ((report.data[9] & 0x0C) >> 2);
		this->z_zero_acceleration = ((report.data[8] & 0xFF) << 2) + (report.data[9] & 0x03);
		qDebug() << "X zero acceleration: " << QString::number(this->x_zero_acceleration, 2);
		qDebug() << "Y zero acceleration: " << QString::number(this->y_zero_acceleration, 2);
		qDebug() << "Z zero acceleration: " << QString::number(this->z_zero_acceleration, 2);

		this->x_gravity = ((report.data[10] & 0xFF) << 2) + ((report.data[13] & 0x30) >> 4);
		this->y_gravity = ((report.data[11] & 0xFF) << 2) + ((report.data[13] & 0x0C) >> 2);
		this->z_gravity = ((report.data[12] & 0xFF) << 2) + (report.data[13] & 0x03);
		qDebug() << "X gravity: " << QString::number(this->x_gravity, 2);
		qDebug() << "Y gravity: " << QString::number(this->y_gravity, 2);
		qDebug() << "Z gravity: " << QString::number(this->z_gravity, 2);

		// Stop checking only calibration reports.
		disconnect(&io_wiimote, SIGNAL(reportReady(QWiimoteReport)), this, SLOT(getCalibrationReport(QWiimoteReport)));
		// Start checking all other reports.
		connect(&io_wiimote, SIGNAL(reportReady(QWiimoteReport)), this, SLOT(getReport(QWiimoteReport)));

		// Start status report polling.
		status_polling = new QTimer(this);
		connect(status_polling, SIGNAL(timeout()), this, SLOT(pollStatusReport()));
		status_polling->start(12000);
		this->pollStatusReport();

	} else {
		qDebug() << "Report ignored.";
		this->requestCalibrationData();
	}
}

/**
  * Gets a report from the wiimote.
  * @param report Received report.
  * @todo Check possible report errors. Use a threshold while reporting acceleration changes.
  */
void QWiimote::getReport(QWiimoteReport report)
{
	int report_type = report.data[0] & 0xFF;

	switch (report_type) {
		case 0x35: // Acceleration + Extension report.
			if (this->data_types & QWiimote::MotionPlusData) {
				qint16 raw_pitch,  raw_roll,  raw_yaw;
				bool   fast_pitch, fast_roll, fast_yaw;

				raw_yaw  =   (report.data[6]  & 0xFF);
				raw_yaw +=   (report.data[9]  & 0xFC) << 6;
				fast_yaw =   (report.data[9]  & 0x02) == 0;

				raw_roll  =  (report.data[7]  & 0xFF);
				raw_roll +=  (report.data[10] & 0xFC) << 6;
				fast_roll =  (report.data[10] & 0x02) == 0;

				raw_pitch  = (report.data[8]  & 0xFF);
				raw_pitch += (report.data[11] & 0xFC) << 6;
				fast_pitch = (report.data[9]  & 0x01) == 0;

				if (this->motionplus_state == QWiimote::MotionPlusWorking) {
					/* Calibrate orientation. Only take into account "still" samples. */
					/** @todo This needs a better method to check that the Wiimote is not moving. */
					if (!fast_pitch &&
						 !fast_roll &&
						 !fast_yaw &&
						 raw_pitch > 7000 && raw_pitch < 9000 &&
						 raw_roll > 7000 && raw_roll < 9000 &&
						 raw_yaw > 7000 && raw_yaw < 9000
						 ) {
						this->pitch_zero_orientation += raw_pitch;
						this->roll_zero_orientation  += raw_roll;
						this->yaw_zero_orientation   += raw_yaw;
						this->num_samples++;

						if (this->calibration_time.elapsed() > QWiimote::MOTIONPLUS_TIME) {

							this->motionplus_state = QWiimote::MotionPlusCalibrated;

							this->pitch_zero_orientation /= this->num_samples;
							this->roll_zero_orientation  /= this->num_samples;
							this->yaw_zero_orientation   /= this->num_samples;

							emit motionPlusState();
						}
					}
				} else {
					qreal yaw_speed, roll_speed, pitch_speed;

					pitch_speed = abs(raw_pitch - this->pitch_zero_orientation) > QWiimote::MOTIONPLUS_THRESHOLD ?
									  raw_pitch - this->pitch_zero_orientation : 0;
					pitch_speed /= (fast_pitch) ?	QWiimote::DEGREES_PER_SECOND_FAST :
															QWiimote::DEGREES_PER_SECOND_SLOW;

					roll_speed = abs(raw_roll - this->roll_zero_orientation) > QWiimote::MOTIONPLUS_THRESHOLD ?
									 raw_roll - this->roll_zero_orientation : 0;
					roll_speed /= (fast_roll) ?	QWiimote::DEGREES_PER_SECOND_FAST :
															QWiimote::DEGREES_PER_SECOND_SLOW;

					yaw_speed = abs(raw_yaw - this->yaw_zero_orientation) > QWiimote::MOTIONPLUS_THRESHOLD ?
									raw_yaw - this->yaw_zero_orientation : 0;
					yaw_speed /= (fast_yaw) ?		QWiimote::DEGREES_PER_SECOND_FAST :
															QWiimote::DEGREES_PER_SECOND_SLOW;

					quint32 elapsed_time = this->last_report.elapsed() - report.time.elapsed();
					qDebug() << "Elapsed time: " << elapsed_time;
					qreal pitch_angle, roll_angle, yaw_angle;
					pitch_angle = (elapsed_time * pitch_speed * QW_PI / 180) / 1000;
					roll_angle = (elapsed_time * roll_speed  * QW_PI / 180) / 1000;
					yaw_angle = (elapsed_time * yaw_speed   * QW_PI / 180) / 1000;

					qDebug() << "Angles: "
							<< pitch_angle << "\t"
							<< roll_angle << "\t"
							<< yaw_angle;

					qreal c_1 = cos(pitch_angle / 2);
					qreal c_2 = cos(roll_angle / 2);
					qreal c_3 = cos(yaw_angle / 2);
					qreal s_1 = sin(pitch_angle / 2);
					qreal s_2 = sin(roll_angle / 2);
					qreal s_3 = sin(yaw_angle / 2);
					QQuaternion new_orientation;

					new_orientation.setX(c_1 * c_2 * c_3 + s_1 * s_2 * s_3);
					new_orientation.setY(s_1 * c_2 * c_3 + c_1 * s_2 * s_3);
					new_orientation.setZ(c_1 * s_2 * c_3 + s_1 * c_2 * s_3);
					new_orientation.setScalar(c_1 * c_2 * s_3 + s_1 * s_2 * c_3);
					new_orientation.normalize();

					this->motionplus_orientation *= new_orientation;
					this->motionplus_orientation.normalize();

					this->last_report = report.time;

					emit this->updatedOrientation();
				}
			}
			// Fallthrough.
		case 0x31: // Acceleration report.
			if (this->data_types & QWiimote::AccelerometerData) {
				quint16 x_new, y_new, z_new;
				x_new =  (report.data[3] & 0xFF) * 4;
				x_new += (report.data[1] & 0x60) >> 5;
				y_new =  (report.data[4] & 0xFF) * 4;
				y_new += (report.data[2] & 0x20) >> 4;
				z_new =  (report.data[5] & 0xFF) * 4;
				z_new += (report.data[2] & 0x40) >> 5;

				// Process acceleration info only if the new values are different than the old ones.
				if ((x_new != this->x_acceleration) ||
						(y_new != this->y_acceleration) ||
						(z_new != this->z_acceleration)) {
					this->x_acceleration = x_new;
					this->y_acceleration = y_new;
					this->z_acceleration = z_new;
					// Calibrated values.
					this->x_calibrated_acceleration = ((qreal)(this->x_acceleration - this->x_zero_acceleration) /
												 (qreal)this->x_gravity);
					this->y_calibrated_acceleration = ((qreal)(this->y_acceleration - this->y_zero_acceleration) /
												 (qreal)this->y_gravity);
					this->z_calibrated_acceleration = ((qreal)(this->z_acceleration - this->z_zero_acceleration) /
												 (qreal)this->z_gravity);
					emit this->updatedAcceleration();
				}
			}
		break;

	case 0x21: // Read memory data, assumed to be a MotionPlus check.
			qDebug() << "Receiving a read memory report " << report.data.toHex() << " from the wiimote.";

			if (((report.data[3] & 0xF0)  != 0xF0) && //There are no errors.
					((report.data[6] & 0xFF)  == 0x00) && //There is a MotionPlus plugged in.
					((report.data[7] & 0xFF)  == 0x00) &&
					((report.data[8] & 0xFF)  == 0xA6) &&
					((report.data[9] & 0xFF)  == 0x20) &&
					((report.data[11] & 0xFF) == 0x05)) {
				qDebug() << "MotionPlus plugged in.";
				if (this->motionplus_state == QWiimote::MotionPlusActivated) {
					this->enableMotionPlus();
					this->motionplus_state = QWiimote::MotionPlusWorking;
					/* Set calibration values to zero. */
					this->pitch_zero_orientation = 0;
					this->roll_zero_orientation  = 0;
					this->yaw_zero_orientation   = 0;
					this->last_report = QTime::currentTime();
					emit motionPlusState();
				}
			}/* else if (this->motionplus_state == QWiimote::MotionPlusWorking) {
				qDebug() << "MotionPlus plugged out.";
				this->motionplus_state = QWiimote::MotionPlusActivated;
				emit motionPlusState(false);
			}*/
		break;

		case 0x20: // Status report.
			qDebug() << "Receiving a status report " << report.data.toHex() << " from the wiimote.";

			quint8 new_battery_level = (report.data[6] & 0xFF);
			bool new_battery_empty = ((report.data[3] & 0x01) == 0x01);
			if (new_battery_level != this->battery_level) {
				this->battery_level = new_battery_level;
				qDebug() << "Battery level: " << this->battery_level;
				emit this->updatedBattery();
			}
			if (new_battery_empty != this->battery_empty) {
				this->battery_empty = new_battery_empty;
				qDebug() << "The battery is now empty.";
				emit this->emptyBattery();
			}

			// If the status request was not requested, the data reporting mode must be changed.
			if (!this->status_requested) {
				this->setDataTypes(this->data_types);
			}
			this->status_requested = false;

		break;
	}

	//Button data is present in every received report for now.
	QWiimote::WiimoteButtons button_new = QFlag(report.data[2] * 0x100 + report.data[1]);
	if (this->button_data != button_new) {
		button_data = button_new;
		emit this->updatedButtons();
	}
	this->last_report = QTime::currentTime();
}

/**
  * Checks if a MotionPlus is connected.
  */
void QWiimote::pollMotionPlus()
{
	send_buffer[0] = 0x17; // Report type.
	send_buffer[1] = 0x04 | (this->led_data & QWiimote::Rumble); // Read from the registers.
	send_buffer[2] = 0xA6; // Memory position.
	send_buffer[3] = 0x00;
	send_buffer[4] = 0xFA;
	send_buffer[5] = 0x00; // Data size.
	send_buffer[6] = 0x06;

	this->io_wiimote.writeReport(send_buffer, 7);
	this->setDataTypes(this->data_types);
}

/**
  * Polls a status report from the Wiimote.
  */
void QWiimote::pollStatusReport()
{
	send_buffer[0] = 0x15; // Report type.
	send_buffer[1] = 0x00 | (this->led_data & QWiimote::Rumble);

	this->io_wiimote.writeReport(send_buffer, 2);
	this->status_requested = true;
}

/**
  * Resets all stored acceleration data.
  */
void QWiimote::resetAccelerationData()
{
	this->x_acceleration = 0;
	this->y_acceleration = 0;
	this->z_acceleration = 0;
	this->x_calibrated_acceleration = 0;
	this->y_calibrated_acceleration = 0;
	this->z_calibrated_acceleration = 0;
}

/**
  * Enable the MotionPlus extension.
  */
void QWiimote::enableMotionPlus()
{
	send_buffer[0] = 0x16;									   // Report type.
	send_buffer[1] = 0x04 | (this->led_data & QWiimote::Rumble); // Write to the registers.
	send_buffer[2] = 0xA6;									   // Memory position.
	send_buffer[3] = 0x00;
	send_buffer[4] = 0xFE;
	send_buffer[5] = 0x01;									   // Data size.
	send_buffer[6] = 0x04;									   // Activate the MotionPlus.

	// Write 0x04 to register 0xA600FE.
	this->io_wiimote.writeReport(send_buffer, 7);

	this->num_samples = 0;
	this->calibration_time = QTime::currentTime();
}

/**
  * Disable the MotionPlus extension.
  */
void QWiimote::disableMotionPlus()
{
	send_buffer[0] = 0x16;									   // Report type.
	send_buffer[1] = 0x04 | (this->led_data & QWiimote::Rumble); // Write to the registers.
	send_buffer[2] = 0xA4;									   // Memory position.
	send_buffer[3] = 0x00;
	send_buffer[4] = 0xF0;
	send_buffer[5] = 0x01;									   // Data size.
	send_buffer[6] = 0x55;									   // Disable the MotionPlus.

	// Write 0x55 to register 0xA400F0.
	this->io_wiimote.writeReport(send_buffer, 7);
}
