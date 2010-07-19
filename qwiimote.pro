# -------------------------------------------------
# Project created by QtCreator 2009-10-20T18:00:49
# -------------------------------------------------
TARGET = qwiimote
TEMPLATE = app
QT += opengl
SOURCES += main.cpp \
    wmainwindow.cpp \
    qwiimote.cpp \
    qiowiimote.cpp \
    wopengl.cpp
HEADERS += wmainwindow.h \
    qwiimote.h \
    debugcheck.h \
    qiowiimote.h \
    qwiimotereport.h \
    wopengl.h
FORMS += wmainwindow.ui
LIBS += libsetupapi \
    libhid
