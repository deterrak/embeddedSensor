QT += core
QT += network
QT -= gui


CONFIG += c++11 -pthread

TARGET = embeddedSensor
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    httppost.cpp \
    gpio.cpp \
    configuration.cpp \
    iot.cpp \
    displaymatrix.cpp \
    irdecoder.cpp

HEADERS += \
    httppost.h \
    gpio.h \
    configuration.h \
    iot.h \
    displaymatrix.h \
    irdecoder.h
