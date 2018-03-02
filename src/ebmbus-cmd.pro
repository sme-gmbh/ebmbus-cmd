QT += core
QT -= gui

CONFIG += c++11

TARGET = ebmbus-cmd
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    revpidio.cpp \
    maincontroller.cpp

LIBS     += -lebmbus

HEADERS += \
    revpidio.h \
    maincontroller.h
