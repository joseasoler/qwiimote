# This file is part of QWiimote.
#
# QWiimote is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# QWiimote is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with QWiimote. If not, see <http://www.gnu.org/licenses/>.

# Compile QWiimote as a library.
TEMPLATE = lib
CONFIG += dll

# Build both debug and release versions.
CONFIG += debug_and_release
CONFIG += build_all
TARGET = $$qtLibraryTarget(QWiimote)

SOURCES += \
    qwiimote.cpp \
    qiowiimote.cpp \
    qprecisetime.cpp

HEADERS += \
    qwiimote.h \
    debugcheck.h \
    qiowiimote.h \
    qwiimotereport.h \
    qprecisetime.h

LIBS += libsetupapi \
    libhid

headers.files = qwiimote.h
headers.path = $$[QT_INSTALL_HEADERS]/qwiimote
INSTALLS += headers

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

dlltarget.path = $$[QT_INSTALL_BINS]
INSTALLS += dlltarget