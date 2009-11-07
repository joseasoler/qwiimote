/**
  * @file qwiimote.cpp
  * Source file for the QWiimote class.
  */

#include "qwiimote.h"
#include "debugcheck.h"

QWiimote::QWiimote(QObject * parent) : QObject(parent), io_wiimote(this)
{
}

QWiimote::~QWiimote()
{
}
