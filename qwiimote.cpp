/**
  * @file
  * Source file for the QWiimote class.
  */

#include "qwiimote.h"
#include <windows.h>
#include <setupapi.h>
#include <ddk/hidsdi.h>

QWiimote::QWiimote()
{
}

bool QWiimote::findWiimote()
{
    return true;
}

