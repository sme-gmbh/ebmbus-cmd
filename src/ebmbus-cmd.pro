#**********************************************************************
#* ebmbus-cmd - a commandline tool to control ebm papst fans
#* Copyright (C) 2018 Smart Micro Engineering GmbH
#* This program is free software: you can redistribute it and/or modify
#* it under the terms of the GNU General Public License as published by
#* the Free Software Foundation, either version 3 of the License, or
#* (at your option) any later version.
#* This program is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#* GNU General Public License for more details.
#* You should have received a copy of the GNU General Public License
#* along with this program. If not, see <http://www.gnu.org/licenses/>.
#*********************************************************************/

QT += core network
QT -= gui

CONFIG += c++11

TARGET = ebmbus-cmd
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

OBJECTS_DIR = .obj/
MOC_DIR = .moc/
UI_DIR = .ui/
RCC_DIR = .rcc/

# make install / make uninstall target
unix {
    target.path = /usr/bin/
    systemdfiles.files += ../unix/ebmbus-cmd.service
    systemdfiles.path = /lib/systemd/system/
    udevfiles.files += ../unix/00-ups500s.rules
    udevfiles.path = /etc/udev/rules.d/
    etcfiles.files += ../etc/ebmbus-cmd.ini.example
    etcfiles.path = /etc/openffucontrol/ebmbus-cmd/
    INSTALLS += target
    INSTALLS += systemdfiles
    INSTALLS += udevfiles
    INSTALLS += etcfiles
}

SOURCES += main.cpp \
    ocu.cpp \
    ocumodbus.cpp \
    ocumodbussystem.cpp \
    revpidio.cpp \
    maincontroller.cpp \
    lightbutton.cpp \
    daisychaininterface.cpp \
    uninterruptiblepowersupply.cpp \
    operatingsystemcontrol.cpp \
    remotecontroller.cpp \
    remoteclienthandler.cpp \
    ffu.cpp \
    ffudatabase.cpp \
    logentry.cpp \
    loghandler.cpp \
    ebmbussystem.cpp \
    ebmmodbussystem.cpp \
    ebmmodbus.cpp \
    auxfandatabase.cpp \
    auxfan.cpp

LIBS     += -lebmbus
LIBS     += -lmodbus

HEADERS += \
    ocu.h \
    ocumodbus.h \
    ocumodbussystem.h \
    revpidio.h \
    maincontroller.h \
    lightbutton.h \
    daisychaininterface.h \
    uninterruptiblepowersupply.h \
    operatingsystemcontrol.h \
    remotecontroller.h \
    remoteclienthandler.h \
    ffu.h \
    ffudatabase.h \
    logentry.h \
    loghandler.h \
    ebmbussystem.h \
    ebmmodbussystem.h \
    ebmmodbus.h \
    auxfandatabase.h \
    auxfan.h

DISTFILES += \
    ../etc/ebmbus-cmd.ini.example \
    ../unix/ebmbus-cmd.service \
    ../unix/00-ups500s.rules

linux-g++: QMAKE_TARGET.arch = $$QMAKE_HOST.arch
linux-g++-32: QMAKE_TARGET.arch = x86
linux-g++-64: QMAKE_TARGET.arch = x86_64
linux-cross: QMAKE_TARGET.arch = x86
win32-cross-32: QMAKE_TARGET.arch = x86
win32-cross: QMAKE_TARGET.arch = x86_64
win32-g++: QMAKE_TARGET.arch = $$QMAKE_HOST.arch
win32-msvc*: QMAKE_TARGET.arch = $$QMAKE_HOST.arch
linux-raspi: QMAKE_TARGET.arch = armv6l
linux-armv6l: QMAKE_TARGET.arch = armv6l
linux-armv7l: QMAKE_TARGET.arch = armv7l
linux-arm*: QMAKE_TARGET.arch = armv6l
linux-aarch64*: QMAKE_TARGET.arch = aarch64

unix {
    equals(QMAKE_TARGET.arch , x86_64): {
        message("Configured for x86_64")
        message("Using libftdi1")
        LIBS +=  -lftdi1
        DEFINES += USE_LIBFTDI1
    }

    equals(QMAKE_TARGET.arch , x86): {
        message("Configured for x86")
        message("Using libftdi1")
        LIBS +=  -lftdi1
        DEFINES += USE_LIBFTDI1
    }

    equals(QMAKE_TARGET.arch , armv6l): {
        message("Configured for armv6l")
        message("Using libftdi")
        LIBS +=  -lftdi
        DEFINES += USE_LIBFTDI
    }

    equals(QMAKE_TARGET.arch , armv7l): {
        message("Configured for armv7l")
        message("Using libftdi")
        LIBS +=  -lftdi
        DEFINES += USE_LIBFTDI
    }
}
