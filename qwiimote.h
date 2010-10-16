/**
  * @file qwiimote.h
  * Header file for the QWiimote class.
  * QWiimote is an API for the Wiimote which follows the Qt Object model.
  * The QWiimote class mantains the state of a single Wiimote.
  * http://qt.nokia.com/doc/4.5/object.html
  */

#ifndef QWIIMOTE_H
#define QWIIMOTE_H

#include <QObject>
#include <QFlags>
#include <QTimer>
#include <QTime>
#include <QMatrix4x4>
#include <QList>
#include "qprecisetime.h"
#include "qiowiimote.h"

/**
  * Stores an acceleration sample.
  * @see #QPreciseTime.
  */
struct QAccelerationSample
{
	QPreciseTime time;                  ///< Time of arrival of the report.
	QVector3D calibrated_acceleration;  ///< Acceleration values.
};

typedef QList<QAccelerationSample> QAccelerationSampleList;

/**
  * QWiimote represents the state of a Wiimote and any connected extensions.
  * @see #QIOWiimote.
  *
  * @todo Using more than one instance of this class is untested.
  */
class QWiimote : public QObject
{
	Q_OBJECT
public:
	/** Defines all data reporting types available. Button data is always active. */
	enum DataType {
		DefaultData       = 0x00, ///< Get only default data (buttons).
		AccelerometerData = 0x01, ///< Get Accelerometer data.
		MotionPlusData    = 0x02, ///< MotionPlus always activates AccelerometerData.
	};

	Q_DECLARE_FLAGS(DataTypes, DataType)

	/** Flags that show if one of the Wiimote buttons has been pressed. */
	enum WiimoteButton {
		ButtonLeft  = 0x0001, ///< Left button of the D-pad.
		ButtonRight = 0x0002, ///< Right button of the D-pad.
		ButtonDown  = 0x0004, ///< Down button of the D-pad.
		ButtonUp    = 0x0008, ///< Up button of the D-pad.
		ButtonPlus  = 0x0010, ///< Plus button.
		ButtonTwo   = 0x0100, ///< Two button.
		ButtonOne   = 0x0200, ///< One button.
		ButtonB     = 0x0400, ///< B button.
		ButtonA     = 0x0800, ///< A button.
		ButtonMinus = 0x1000, ///< Minus button.
		ButtonHome  = 0x8000, ///< Home button.
	};

	Q_DECLARE_FLAGS(WiimoteButtons, WiimoteButton)

	/** Smoothing method used for determining acceleration. */
	enum AccelerationSmoothing {
		SmoothingNone, ///< Use the last sample.
		SmoothingEMA,  ///< Apply Exponential Moving Average to the list of samples.
	};

	Q_DECLARE_FLAGS(AccSmoothing, AccelerationSmoothing)

	/** State of the MotionPlus extension. */
	enum MotionPlusState {
		MotionPlusInactive,   ///< Initial state.
		MotionPlusActivated,  ///< The MotionPlus has been activated by the user, but not by hardware.
		MotionPlusWorking,    ///< The MotionPlus is working but it is not calibrated.
		MotionPlusCalibrated, ///< The MotionPlus is calibrated and its data is now used.
	};

	Q_DECLARE_FLAGS(MotionPlusStates, MotionPlusState)

	/** Flags that show if the Wiimote leds / rumble are active. */
	enum WiimoteLed {
		Rumble = 0x01, ///< Rumble is on.
		Led1   = 0x10, ///< Led 1 is on.
		Led2   = 0x20, ///< Led 2 is on.
		Led3   = 0x40, ///< Led 3 is on.
		Led4   = 0x80, ///< Led 4 is on.
	};

	Q_DECLARE_FLAGS(WiimoteLeds, WiimoteLed)

	QWiimote(QObject * parent = NULL);
	~QWiimote();
	bool start(QWiimote::DataTypes new_data_types = QWiimote::DefaultData);
	void stop();

	void setDataTypes(QWiimote::DataTypes new_data_types);
	void setLeds(QWiimote::WiimoteLeds leds);

	void setAccelerationSmoothing(QWiimote::AccelerationSmoothing acc_s);

	QWiimote::DataTypes dataTypes() const;
	QWiimote::WiimoteLeds leds() const;
	QWiimote::WiimoteButtons buttonData() const;

	QVector3D rawAcceleration() const;
	QVector3D acceleration() const;

	QMatrix4x4 orientation() const { return mat_orientation; }

	quint8 batteryLevel() const;
	bool batteryEmpty() const;

signals:
	/** Emitted when the state of the buttons changes. */
	void updatedButtons();
	/** Emitted when the acceleration values change. */
	void updatedAcceleration();
	/** Emitted when the state of the battery changes. */
	void updatedBattery();
	/** Emitted when the battery is empty. */
	void emptyBattery();
	/** Emitted when the MotionPlus changes its state. */
	void motionPlusState();
	/** Emitted when the orientation values change. */
	void updatedOrientation();
private:
	bool requestCalibrationData();
	void resetAccelerationData();
	void enableMotionPlus();
	void disableMotionPlus();
	void processOrientationData();

	static const quint8 SMOOTHING_NONE_THRESHOLD; ///< Raw acceleration threshold for non-smoothed data.
	static const qreal  SMOOTHING_EMA_THRESHOLD;  ///< Calibrated acceleration threshold for EMA.
	static const quint16 MOTIONPLUS_TIME;         ///< Time required to calibrate the MotionPlus.
	static const qreal DEGREES_PER_SECOND_SLOW;   ///< MotionPlus speed (slow).
	static const qreal DEGREES_PER_SECOND_FAST;   ///< MotionPlus speed (fast).
	static const quint8 MOTIONPLUS_THRESHOLD;     ///< MotionPlus speed threshold.

	QIOWiimote io_wiimote;                ///< Instance of QIOWiimote used to send / receive wiimote data.
	char send_buffer[22];                 ///< Buffer used to send reports to the wiimote.

	QWiimote::DataTypes data_types;       ///< Current data type status.
	QWiimote::WiimoteButtons button_data; ///< Button status.
	QWiimote::WiimoteLeds led_data;       ///< Led status.

	QPreciseTime last_report;             ///< Time when the last report was received.

	/* Raw acceleration values. */
	QVector3D raw_acceleration;           ///< Raw acceleration vector.
	/* Acceleration calibration values. */
	QVector3D zero_acceleration;          ///< Zero position for the accelerometer.
	QVector3D gravity;                    ///< Gravity calibration for the accelerometer.
	/* Acceleration samples. */
	QAccelerationSampleList sample_list;  ///< List of acceleration samples.
	QWiimote::AccelerationSmoothing
					acceleration_smoothing;   ///< Method used for smoothing the acceleration value.
	quint8 max_acceleration_samples;      ///< Maximum number of acceleration samples to store.
	/* Current acceleration values. */
	QVector3D calibrated_acceleration;    ///< Acceleration vector.

	QTimer * motionplus_polling;          ///< Timer that checks the MotionPlus state.
	QWiimote::MotionPlusStates
					motionplus_state;         ///< Current state of the MotionPlus.

	QMatrix4x4 mat_orientation;           ///< Orientation of the Wiimote.
	QTime calibration_time;               ///< Used to calibrate orientation for a certain amount of time.
	quint16 calibration_samples;          ///< Number of samples taken for calibrating the orientation.
	qint32 pitch_zero_orientation;        ///< Zero angle for pitch.
	qint32 roll_zero_orientation;         ///< Zero angle for roll.
	qint32 yaw_zero_orientation;          ///< Zero angle for yaw.
	qreal pitch_speed;                    ///< Pitch speed (in degrees per second).
	qreal roll_speed;                     ///< Roll speed (in degrees per second).
	qreal yaw_speed;                      ///< Yaw speed (in degrees per second).
	qreal elapsed_time;                   ///< Elapsed time from last report (in milliseconds).

	QTimer * status_polling;              ///< Timer that polls wiimote status reports.
	bool status_requested;                ///< True if a status report is expected.
	quint8 battery_level;                 ///< Battery level of the wiimote.
	bool battery_empty;                   ///< True if the battery is almost empty.


private slots:
	void getCalibrationReport(QWiimoteReport report);
	void getReport(QWiimoteReport report);
	void pollMotionPlus();
	void pollStatusReport();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::DataTypes)

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::WiimoteButtons)

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::WiimoteLeds)

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::MotionPlusStates)

#endif // QWIIMOTE_H
