#-------------------------------------------------
#
# Project created by QtCreator 2018-09-13T15:29:42
#
#-------------------------------------------------

QT       += core gui widgets network concurrent

TARGET = untitled2
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    tcpclient.cpp \
    file.cpp \
    wcdma.cpp \
    lan.cpp \
    clickevent.cpp \
    inverter_stp25k.cpp \
    inverter_stp50k.cpp \
    inverter_sc1m.cpp \
    inverter_stp60k.cpp

HEADERS += \
        mainwindow.h \
    data.h \
    tcpclient.h

FORMS += \
        mainwindow.ui

INCLUDEPATH    += /usr/local/include
LIBS += -L/usr/local/lib -lwiringPi

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
