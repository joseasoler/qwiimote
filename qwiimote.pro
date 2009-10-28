# -------------------------------------------------
# Project created by QtCreator 2009-10-20T18:00:49
# -------------------------------------------------
TARGET = qwiimote
TEMPLATE = app
SOURCES += main.cpp \
    wmainwindow.cpp \
    qwiimote.cpp
HEADERS += wmainwindow.h \
    qwiimote.h \
    debugcheck.h
FORMS += wmainwindow.ui
LIBS += libsetupapi \
    libhid
