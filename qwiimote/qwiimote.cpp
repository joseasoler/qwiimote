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
 * @file qwiimote.cpp
 *
 * Source file for the QWiimote class.
 */

#include <cmath>
#include "qwiimote.h"
#include "debugcheck.h"
#include "qprecisetime.h"
#include "qiowiimote.h"
#include "qwiimotereport.h"

/**
 * Stores an acceleration sample.
 * @see #QPreciseTime.
 */
struct QAccelerationSample
{
	QPreciseTime time;                  ///< Time of arrival of the report.
	QVector3D calibrated_acceleration;  ///< Acceleration values.
};

const quint8  QWiimote::SMOOTHING_NONE_THRESHOLD = 3;
const qreal   QWiimote::SMOOTHING_EMA_THRESHOLD = 0.01;
const quint16 QWiimote::MOTIONPLUS_TIME = 8000;
const qreal   QWiimote::DEGREES_PER_SECOND_SLOW = 8192.0 / 595.0;
const qreal   QWiimote::DEGREES_PER_SECOND_FAST = QWiimote::DEGREES_PER_SECOND_SLOW / 2000 / 440;

#define QW_PI (3.141592653589793238462643) ///< Pi constant.
#define QW_RAD_TO_DEGREES(angle) (angle * 180 / QW_PI)

/**
 * Creates a new QWiimote instance.
 * @param parent The parent of this instance.
 */
QWiimote::QWiimote(QObject * parent) : QObject(parent)
{
	io_wiimote  = new QIOWiimote(this);
	last_report = new QPreciseTime();
}

/**
 * Destructor of QWiimote.
 */
QWiimote::~QWiimote()
{
	this->stop();
}

/**
 * The QWiimote starts working.
 * @return true if the QWiimote started correctly.
 */
bool QWiimote::start(QWiimote::DataTypes new_data_types)
{
	if (this->io_wiimote->open()) {
		/* All reports are ignored until the calibration data is received. */
		connect(io_wiimote, SIGNAL(reportReady(QWiimoteReport *)), this, SLOT(getCalibrationReport(QWiimoteReport *)));
		this->requestCalibrationData();
		/* Initialize internal values. */
		data_types = 0;
		this->status_requested = false;
		this->battery_level = 0;
		this->battery_empty = false;
		this->acceleration_smoothing = QWiimote::SmoothingEMA;
		this->max_acceleration_samples = 24;
		this->motionplus_threshold = 30;

		this->orientation_mode = QWiimote::OrientationModeNone;
		this->orientation_matrix = NULL;
		this->pitch_orientation = 0;
		this->roll_orientation = 0;
		this->yaw_orientation = 0;

		this->motionplus_state = QWiimote::MotionPlusInactive;
		this->motionplus_polling = NULL;
		this->pitch_speed = 0;
		this->roll_speed = 0;
		this->yaw_speed = 0;

		this->setDataTypes(new_data_types);

		return true;
	}

	return false;
}

/**
 * The QWiimote stops working and disconnects.
 */
void QWiimote::stop()
{
	this->setDataTypes(QWiimote::DefaultData);
	this->io_wiimote->close();
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
		/* MotionPlus always activates AccelerometerData. */
		new_data_types |= QWiimote::AccelerometerData;
		this->motionplus_state = QWiimote::MotionPlusActivated;

		/* Start MotionPlus polling. */
		if (this->motionplus_polling == NULL) {
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

	send_buffer[0] = 0x12;

	if ((this->data_types & QWiimote::MotionPlusData) != 0 &&
		(this->motionplus_state == QWiimote::MotionPlusWorking ||
		 this->motionplus_state == QWiimote::MotionPlusCalibrated)) {
		/* Continuous reporting required. */
		send_buffer[1] = 0x04;
		send_buffer[2] = 0x35;
	} else if ((this->data_types & QWiimote::AccelerometerData) != 0 && this->motionplus_state != QWiimote::MotionPlusWorking) {
		/* Continuous reporting required. */
		send_buffer[1] = 0x04;
		send_buffer[2] = 0x31;
	}
	else {
		/* Continuous reporting not required. */
		send_buffer[1] = 0x00;
		send_buffer[2] = 0x30;
		resetAccelerationData();
	}

	send_buffer[1] |= this->led_data & QWiimote::Rumble;

	this->io_wiimote->writeReport(send_buffer, 3);
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
	this->io_wiimote->writeReport(send_buffer, 2);
}

/**
 * Modify the calibration values for the accelerometer.
 * @param acc_s Smoothing method to use.
 */
void QWiimote::setAccelerationCalibration(QVector3D zero_acc, QVector3D grav)
{
	this->zero_acceleration = zero_acc;
	this->gravity = grav;
}

/**
 * Changes the method used for smoothing acceleration samples.
 * @param acc_s Smoothing method to use.
 */
void QWiimote::setAccelerationSmoothing(QWiimote::AccelerationSmoothing acc_s)
{
	if (this->acceleration_smoothing == acc_s) return;

	this->sample_list.clear();
	this->acceleration_smoothing = acc_s;
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
 * Gets the current raw acceleration value.
 * @return Raw acceleration vector.
 */
QVector3D QWiimote::rawAcceleration() const
{
	return this->raw_acceleration;
}

/**
 * Gets the current calibrated acceleration value.
 * @return Calibrated acceleration.
 */
QVector3D QWiimote::acceleration() const
{
	return this->calibrated_acceleration;
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
 * Is the wiimote still?
 * @return True iff the wiimote accelerometer is still.
 */
bool QWiimote::isStill() const
{
	/* We cannot know if the Wiimote is still in this mode. */
	if (this->acceleration_smoothing == QWiimote::SmoothingNone) return false;

	QAccelerationSampleList::const_iterator last = this->sample_list.end();
	last--;

	QVector3D still = this->calibrated_acceleration - last->calibrated_acceleration;

	return (abs(still.x()) <= QWiimote::SMOOTHING_EMA_THRESHOLD &&
			abs(still.y()) <= QWiimote::SMOOTHING_EMA_THRESHOLD &&
			abs(still.z()) <= QWiimote::SMOOTHING_EMA_THRESHOLD);
}

/**
 * Request the calibration data from the Wiimote.
 * @return True if the report was sent correctly.
 */
bool QWiimote::requestCalibrationData()
{
	send_buffer[0] = 0x17; // Report type.
	send_buffer[1] = 0x00 | (this->led_data & QWiimote::Rumble); // Read from the EEPROM.
	send_buffer[2] = 0x16; // Memory position.
	send_buffer[3] = 0x00;
	send_buffer[4] = 0x00;
	send_buffer[5] = 0x00; // Data size.
	send_buffer[6] = 0x20;

	return this->io_wiimote->writeReport(send_buffer, 7);
}

/**
 * This function processes all incoming reports until a report with the acceleration data is received.
 * @param report Received report.
 */
void QWiimote::getCalibrationReport(QWiimoteReport *report)
{
	// qDebug() << "Receiving the report " << report->data.toHex() << " from the wiimote.";
	if (report->data[0] == (char)0x21) {
		qDebug() << "Calibration report:" << report->data.toHex();
		/* Get the required calibration values from the report. */
		this->zero_acceleration.setX(((report->data[12] & 0xFF) << 2) + ((report->data[15] & 0x30) >> 4));
		this->zero_acceleration.setY(((report->data[14] & 0xFF) << 2) +  (report->data[15] & 0x03));
		this->zero_acceleration.setZ(((report->data[13] & 0xFF) << 2) + ((report->data[15] & 0x0C) >> 2));

		this->gravity.setX(((report->data[16] & 0xFF) << 2) + ((report->data[19] & 0x30) >> 4));
		this->gravity.setY(((report->data[18] & 0xFF) << 2) +  (report->data[19] & 0x03));
		this->gravity.setZ(((report->data[17] & 0xFF) << 2) + ((report->data[19] & 0x0C) >> 2));
		this->gravity -= this->zero_acceleration;

		/* Stop checking only calibration reports. */
		disconnect(io_wiimote, SIGNAL(reportReady(QWiimoteReport *)), this, SLOT(getCalibrationReport(QWiimoteReport *)));
		// Start checking all other reports.
		connect(io_wiimote, SIGNAL(reportReady(QWiimoteReport *)), this, SLOT(getReport(QWiimoteReport *)));

		/* Start status report polling. */
		status_polling = new QTimer(this);
		connect(status_polling, SIGNAL(timeout()), this, SLOT(pollStatusReport()));
		status_polling->start(12000);
		this->pollStatusReport();
	} else {
		this->requestCalibrationData();
	}
}

/**
 * Gets a report from the wiimote.
 * @param report Received report.
 * @todo Check possible report errors.
 */
void QWiimote::getReport(QWiimoteReport *report)
{
	int report_type = report->data[0] & 0xFF;

	switch (report_type) {
		case 0x35: // Acceleration + Extension report.
			if (this->data_types & QWiimote::MotionPlusData) {
				qint16 raw_pitch,  raw_roll,  raw_yaw;
				bool   fast_pitch, fast_roll, fast_yaw;

				raw_yaw  =   (report->data[6]  & 0xFF);
				raw_yaw +=   (report->data[9]  & 0xFC) << 6;
				fast_yaw =   (report->data[9]  & 0x02) == 0;

				raw_roll  =  (report->data[7]  & 0xFF);
				raw_roll +=  (report->data[10] & 0xFC) << 6;
				fast_roll =  (report->data[10] & 0x02) == 0;

				raw_pitch  = (report->data[8]  & 0xFF);
				raw_pitch += (report->data[11] & 0xFC) << 6;
				fast_pitch = (report->data[9]  & 0x01) == 0;

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
						this->motionplus_calibration_samples++;

						if (this->motionplus_calibration_time.elapsed() > QWiimote::MOTIONPLUS_TIME) {

							this->motionplus_state = QWiimote::MotionPlusCalibrated;

							this->pitch_zero_orientation /= this->motionplus_calibration_samples;
							this->roll_zero_orientation  /= this->motionplus_calibration_samples;
							this->yaw_zero_orientation   /= this->motionplus_calibration_samples;

							emit motionPlusState(this->motionplus_state);
						}
					}
				} else {
					pitch_speed = abs(raw_pitch - this->pitch_zero_orientation) > this->GetMotionPlusThreshold() ?
									  raw_pitch - this->pitch_zero_orientation : 0;
					pitch_speed /= (fast_pitch) ?	QWiimote::DEGREES_PER_SECOND_FAST :
															QWiimote::DEGREES_PER_SECOND_SLOW;

					roll_speed = abs(raw_roll - this->roll_zero_orientation) > this->GetMotionPlusThreshold() ?
									 raw_roll - this->roll_zero_orientation : 0;
					roll_speed /= (fast_roll) ?	QWiimote::DEGREES_PER_SECOND_FAST :
															QWiimote::DEGREES_PER_SECOND_SLOW;

					yaw_speed = abs(raw_yaw - this->yaw_zero_orientation) > this->GetMotionPlusThreshold() ?
									raw_yaw - this->yaw_zero_orientation : 0;
					yaw_speed /= (fast_yaw) ?		QWiimote::DEGREES_PER_SECOND_FAST :
															QWiimote::DEGREES_PER_SECOND_SLOW;

					this->elapsed_time = this->last_report->elapsed() - report->time.elapsed();
					(*this->last_report) = report->time;
				}
			}
			/* FALL THROUGH */
		case 0x31: // Acceleration report.
			if (this->data_types & QWiimote::AccelerometerData) {
				quint16 x_new, y_new, z_new;
				x_new =  (report->data[3] & 0xFF) << 2;
				x_new += (report->data[1] & 0x60) >> 5;
				y_new =  (report->data[5] & 0xFF) << 2;
				y_new += (report->data[2] & 0x40) >> 5;
				z_new =  (report->data[4] & 0xFF) << 2;
				z_new += (report->data[2] & 0x20) >> 4;

				if (this->acceleration_smoothing != QWiimote::SmoothingNone) {
					this->raw_acceleration = QVector3D(x_new, y_new, z_new);
					/* Add the new sample to the beginning of the list of samples. */
					QAccelerationSample sample;
					sample.time = report->time;
					/* Calibrated values. */
					sample.calibrated_acceleration = this->raw_acceleration - this->zero_acceleration;
					sample.calibrated_acceleration.setX(sample.calibrated_acceleration.x() / this->gravity.x());
					sample.calibrated_acceleration.setY(sample.calibrated_acceleration.y() / this->gravity.y());
					sample.calibrated_acceleration.setZ(sample.calibrated_acceleration.z() / this->gravity.z());
					this->sample_list.prepend(sample);

					/* Remove the last sample if required. */
					if (this->sample_list.size() > this->max_acceleration_samples) this->sample_list.removeLast();
				}


				switch (this->acceleration_smoothing) {
					case QWiimote::SmoothingNone:
						/* Process acceleration info only if the new values are different than the old ones. */
						if (	(abs(x_new - this->raw_acceleration.x()) > QWiimote::SMOOTHING_NONE_THRESHOLD) ||
								(abs(y_new - this->raw_acceleration.y()) > QWiimote::SMOOTHING_NONE_THRESHOLD) ||
								(abs(z_new - this->raw_acceleration.z()) > QWiimote::SMOOTHING_NONE_THRESHOLD)) {
							this->raw_acceleration = QVector3D(x_new, y_new, z_new);
							this->calibrated_acceleration = this->raw_acceleration - this->zero_acceleration;
							this->calibrated_acceleration.setX(this->calibrated_acceleration.x() / this->gravity.x());
							this->calibrated_acceleration.setY(this->calibrated_acceleration.y() / this->gravity.y());
							this->calibrated_acceleration.setZ(this->calibrated_acceleration.z() / this->gravity.z());

							emit this->updatedAcceleration();
						}
						break;

					case QWiimote::SmoothingEMA:
						/* Exponential Moving Average method. */
						qreal alpha = 0.1;
						qreal one_minus_alpha = 1.0 - alpha;
						qreal alpha_pow = 1;

						QVector3D EMA_acceleration(0.0, 0.0, 0.0);

						for (QAccelerationSampleList::iterator i = this->sample_list.begin();
								i != this->sample_list.end();
								i++) {
							EMA_acceleration += i->calibrated_acceleration * alpha_pow;
							alpha_pow *= one_minus_alpha;
						}

						EMA_acceleration *= alpha;
						EMA_acceleration.normalize();

						if (	(fabs(EMA_acceleration.x() - this->calibrated_acceleration.x()) > QWiimote::SMOOTHING_EMA_THRESHOLD) ||
								(fabs(EMA_acceleration.y() - this->calibrated_acceleration.y()) > QWiimote::SMOOTHING_EMA_THRESHOLD) ||
								(fabs(EMA_acceleration.z() - this->calibrated_acceleration.z()) > QWiimote::SMOOTHING_EMA_THRESHOLD)) {
							this->calibrated_acceleration = EMA_acceleration;
							emit this->updatedAcceleration();
						}
						break;
				}

				this->processOrientationData();
			}
		break;

	case 0x21: // Read memory data, assumed to be a MotionPlus check.
			// qDebug() << "Receiving a read memory report " << report->data.toHex() << " from the wiimote.";

			if (((report->data[3] & 0xF0)  != 0xF0) && //There are no errors.
					((report->data[6] & 0xFF)  == 0x00) && //There is a MotionPlus plugged in.
					((report->data[7] & 0xFF)  == 0x00) &&
					((report->data[8] & 0xFF)  == 0xA6) &&
					((report->data[9] & 0xFF)  == 0x20) &&
					((report->data[11] & 0xFF) == 0x05)) {
				if (this->motionplus_state == QWiimote::MotionPlusActivated) {
					this->enableMotionPlus();
					this->motionplus_state = QWiimote::MotionPlusWorking;
					/* Set calibration values to zero. */
					this->pitch_zero_orientation = 0;
					this->roll_zero_orientation  = 0;
					this->yaw_zero_orientation   = 0;
					(*this->last_report) = QPreciseTime::currentTime();
					emit motionPlusState(this->motionplus_state);
				}
			}
		break;

		case 0x20: // Status report.
			// qDebug() << "Receiving a status report " << report->data.toHex() << " from the wiimote.";

			quint8 new_battery_level = (report->data[6] & 0xFF);
			bool new_battery_empty = ((report->data[3] & 0x01) == 0x01);
			/* Check if the battery level has changed. */
			if (new_battery_level != this->battery_level) {
				this->battery_level = new_battery_level;
				emit this->updatedBattery();
			}

			/* Check if the battery is almost empty. */
			if (new_battery_empty != this->battery_empty) {
				this->battery_empty = new_battery_empty;
				emit this->emptyBattery();
			}

			/* If the status request was not requested, the data reporting mode must be changed. */
			if (!this->status_requested) {
				this->setDataTypes(this->data_types);
			}
			this->status_requested = false;

		break;
	}

	/* Button data is present in every received report for now. */
	QWiimote::WiimoteButtons button_new = QFlag(report->data[2] * 0x100 + report->data[1]);
	if (this->button_data != button_new) {
		button_data = button_new;
		emit this->updatedButtons();
	}
	(*this->last_report) = QPreciseTime::currentTime();
}

/**
 * Set the current #OrientationMode.
 * @param new_mode New mode to use.
 */
void QWiimote::setOrientationMode(QWiimote::OrientationMode new_mode)
{
	this->orientation_mode = new_mode;
}

/**
 * Allows to know the current #OrientationMode.
 * @return #OrientationMode currently in use.
 */
QWiimote::OrientationMode QWiimote::getOrientationMode() const
{
	return this->orientation_mode;
}


/**
 * If necessary, prepare the orientation matrix for being used.
 */
void QWiimote::PrepareOrientationMatrix()
{
	if (this->orientation_matrix != NULL) return;

	this->orientation_matrix = new QMatrix4x4();
	this->orientation_matrix->setToIdentity();
}

/**
 * Update a orientation matrix with the given data.
 */
void UpdateOrientationMatrix(QMatrix4x4 &matrix, qreal pitch_change, qreal roll_change, qreal yaw_change)
{
	/* Order of application: http://www.euclideanspace.com/maths/geometry/rotations/euler/index.htm */
	matrix.rotate(-yaw_change,   0.0, 1.0, 0.0);
	matrix.rotate( pitch_change, 1.0, 0.0, 0.0);
	matrix.rotate(-roll_change,  0.0, 0.0, 1.0);
}

/**
 * Get the pitch and roll angles from accelerometer data.
 * @todo Currently, transition fails between some quadrants. This means
 * that the method currently being used for calculating angles is wrong.
 */
void QWiimote::GetAnglesFromAccelerometer(qreal &final_pitch, qreal &final_roll)
{
	if (!(this->data_types & QWiimote::AccelerometerData)) return;

	/* Use accelerometer data to determine pitch and roll. */
	QVector3D acc = -this->acceleration();
	acc.normalize();

	/* http://code.google.com/p/giimote/wiki/Pitch */
	qreal pitch = QW_RAD_TO_DEGREES(atan2(acc.z(), sqrt(acc.x() * acc.x() + acc.y() * acc.y())));
	/* http://code.google.com/p/giimote/wiki/Roll */
	qreal roll =  QW_RAD_TO_DEGREES(atan2(acc.x(), sqrt(acc.z() * acc.z() + acc.y() * acc.y())));

	if        (acc.x() >= 0 && acc.z() >= 0 && acc.y() <  0) {
		final_pitch = pitch;
		final_roll  = roll;
	} else if (acc.x() <  0 && acc.z() >= 0 && acc.y() <  0) {
		final_pitch = pitch;
		final_roll  = roll;
	} else if (acc.x() <  0 && acc.z() >= 0 && acc.y() >= 0) {
		final_pitch = 180.0 - pitch;
		final_roll  = roll;
	} else if (acc.x() >= 0 && acc.z() >= 0 && acc.y() >= 0) {
		final_pitch = 180.0 - pitch;
		final_roll  = roll;
	} else if (acc.x() <  0 && acc.z() <  0 && acc.y() >= 0) {
		final_pitch = 180.0 - pitch;
		final_roll  = roll;
	} else if (acc.x() >= 0 && acc.z() <  0 && acc.y() >= 0) {
		final_pitch = 180.0 - pitch;
		final_roll  = roll;
	} else if (acc.x() >= 0 && acc.z() <  0 && acc.y() <  0) {
		final_pitch = pitch;
		final_roll  = roll;
	} else if (acc.x() <  0 && acc.z() <  0 && acc.y() <  0) {
		final_pitch = pitch;
		final_roll  = roll;
	}
}

/**
 * Turns raw data into a matrix that measures current orientation of the wiimote.
 * @todo The constant 0.65 has just been tweaked until it seems to work, but it is not exact at all. Proper measurements should be made.
 */
void QWiimote::processOrientationData()
{
	this->pitch_orientation = 0;
	this->roll_orientation  = 0;
	this->yaw_orientation   = 0;
	/* Do nothing. */
	if (this->orientation_mode == QWiimote::OrientationModeNone) return;

	qreal pitch_change = 0;
	qreal roll_change  = 0;
	qreal yaw_change   = 0;

	if (this->motionplus_state == QWiimote::MotionPlusCalibrated) {
		pitch_change = -0.65 * (elapsed_time * this->pitch_speed) / 1000;
		roll_change  = -0.65 * (elapsed_time * this->roll_speed)  / 1000;
		yaw_change   = -0.65 * (elapsed_time * this->yaw_speed)   / 1000;
	}

	this->PrepareOrientationMatrix();

	switch (this->orientation_mode) {
		case QWiimote::OrientationModeRaw:
			if (this->motionplus_state == QWiimote::MotionPlusCalibrated) {
				UpdateOrientationMatrix(*this->orientation_matrix, pitch_change, roll_change, yaw_change);
			} else if (!(this->data_types & QWiimote::MotionPlusData)) {
				this->GetAnglesFromAccelerometer(this->pitch_orientation, this->roll_orientation);
			}
			break;

		case QWiimote::OrientationModeMixed: {
			if (this->motionplus_state != QWiimote::MotionPlusCalibrated) return;
			this->GetAnglesFromAccelerometer(this->pitch_orientation, this->roll_orientation);
			UpdateOrientationMatrix(*this->orientation_matrix, pitch_change, roll_change, yaw_change);
			/* Conversion from matrix to angles: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToEuler/index.htm */
			const qreal *matrix_data = this->orientation_matrix->constData();
			qreal m10 = matrix_data[0 + 1 * 4];

			if (m10 > 0.998 || m10 < -0.998) {
				qreal m02 = matrix_data[2 + 0 * 4];
				qreal m22 = matrix_data[2 + 2 * 4];
				this->yaw_orientation = atan2( m02, m22);
			} else {
				qreal m20 = matrix_data[0 + 2 * 4];
				qreal m00 = matrix_data[0 + 0 * 4];
				this->yaw_orientation = atan2(-m20, m00);
			}

			this->yaw_orientation = QW_RAD_TO_DEGREES(this->yaw_orientation);
			break;
		}
	}

	emit this->updatedOrientation();
}

/**
 * Get a matrix with orientation data. See #OrientationMode.
 * @return Orientation matrix.
 */
QMatrix4x4 QWiimote::orientation() const
{
	if (this->orientation_matrix == NULL ||
		this->orientation_mode == QWiimote::OrientationModeNone) return QMatrix4x4();

	switch (this->orientation_mode) {
		case QWiimote::OrientationModeRaw:
			return QMatrix4x4(*this->orientation_matrix);

		case QWiimote::OrientationModeMixed:
			QMatrix4x4 calculated_matrix;
			UpdateOrientationMatrix(calculated_matrix, this->pitch_orientation, -this->roll_orientation, this->yaw_orientation);
			return calculated_matrix;
	}

	return QMatrix4x4();
}

/**
 * Resets the MotionPlus orientation data.
 */
void QWiimote::resetOrientation()
{
	if (this->orientation_matrix == NULL) return;
	this->orientation_matrix->setToIdentity();
}

/**
 * Checks if a MotionPlus is connected.
 */
void QWiimote::pollMotionPlus()
{
	send_buffer[0] = (char)0x17; // Report type.
	send_buffer[1] = (char)0x04 | (this->led_data & QWiimote::Rumble); // Read from the registers.
	send_buffer[2] = (char)0xA6; // Memory position.
	send_buffer[3] = (char)0x00;
	send_buffer[4] = (char)0xFA;
	send_buffer[5] = (char)0x00; // Data size.
	send_buffer[6] = (char)0x06;

	this->io_wiimote->writeReport(send_buffer, 7);
	this->setDataTypes(this->data_types);
}

/**
 * Polls a status report from the Wiimote.
 */
void QWiimote::pollStatusReport()
{
	send_buffer[0] = (char)0x15; // Report type.
	send_buffer[1] = (char)0x00 | (this->led_data & QWiimote::Rumble);

	this->io_wiimote->writeReport(send_buffer, 2);
	this->status_requested = true;
}

/**
 * Resets all stored acceleration data.
 */
void QWiimote::resetAccelerationData()
{
	this->sample_list.clear();
	this->raw_acceleration = QVector3D(0.0, 0.0, 0.0);
	this->calibrated_acceleration = QVector3D(0.0, 0.0, 0.0);
}

/**
 * Enable the MotionPlus extension.
 */
void QWiimote::enableMotionPlus()
{
	send_buffer[0] = (char)0x16;									   // Report type.
	send_buffer[1] = (char)0x04 | (this->led_data & QWiimote::Rumble); // Write to the registers.
	send_buffer[2] = (char)0xA6;									   // Memory position.
	send_buffer[3] = (char)0x00;
	send_buffer[4] = (char)0xFE;
	send_buffer[5] = (char)0x01;									   // Data size.
	send_buffer[6] = (char)0x04;									   // Activate the MotionPlus.

	// Write 0x04 to register 0xA600FE.
	this->io_wiimote->writeReport(send_buffer, 7);

	this->motionplus_calibration_samples = 0;
	this->motionplus_calibration_time = QTime::currentTime();
}

/**
 * Disable the MotionPlus extension.
 */
void QWiimote::disableMotionPlus()
{
	send_buffer[0] = (char)0x16;									   // Report type.
	send_buffer[1] = (char)0x04 | (this->led_data & QWiimote::Rumble); // Write to the registers.
	send_buffer[2] = (char)0xA4;									   // Memory position.
	send_buffer[3] = (char)0x00;
	send_buffer[4] = (char)0xF0;
	send_buffer[5] = (char)0x01;									   // Data size.
	send_buffer[6] = (char)0x55;									   // Disable the MotionPlus.

	// Write 0x55 to register 0xA400F0.
	this->io_wiimote->writeReport(send_buffer, 7);
}
