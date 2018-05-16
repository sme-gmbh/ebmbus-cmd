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
    INSTALLS += target
    INSTALLS += systemdfiles
}


SOURCES += main.cpp \
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
    ebmbussystem.cpp

LIBS     += -lebmbus

HEADERS += \
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
    ebmbussystem.h

DISTFILES += \
    ../unix/ebmbus-cmd.service
