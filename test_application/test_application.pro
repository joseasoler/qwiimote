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

TARGET = test_application
TEMPLATE = app
QT += opengl

SOURCES += main.cpp \
	wmainwindow.cpp \
	wopengl.cpp

HEADERS += wmainwindow.h \
	wopengl.h

FORMS += wmainwindow.ui

LIBS += libsetupapi \
	libhid

# Use a different library for Debug/Release.
debug {
	LIBS += libQWiimoted
} else {
	LIBS += libQWiimote
}
