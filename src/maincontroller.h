/**********************************************************************
** ebmbus-cmd - a commandline tool to control ebm papst fans
** Copyright (C) 2018 Smart Micro Engineering GmbH
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <libebmbus/ebmbus.h>
#include "revpidio.h"
#include "ebmbussystem.h"
#include "ebmmodbussystem.h"
#include "ocumodbussystem.h"
#include "lightbutton.h"
#include "uninterruptiblepowersupply.h"
#include "operatingsystemcontrol.h"
#include "remotecontroller.h"
#include "ffudatabase.h"
#include "auxfandatabase.h"
#include "ocudatabase.h"
#include "loghandler.h"

class MainController : public QObject
{
    Q_OBJECT

public:
    explicit MainController(QObject *parent = nullptr);
    ~MainController();

private:
    Loghandler* m_loghandler;
    QSettings* m_settings;

    RevPiDIO m_io;

    LightButton* m_lightbutton_operation;
    LightButton* m_lightbutton_error;
    LightButton* m_lightbutton_speed_0;
    LightButton* m_lightbutton_speed_50;
    LightButton* m_lightbutton_speed_100;

    EbmBusSystem* m_ebmbusSystem;
    EbmModbusSystem* m_ebmModbusSystem;
    OcuModbusSystem* m_ocuModbusSystem;

    UninterruptiblePowerSupply* m_ups;
    OperatingSystemControl* m_osControl;

    FFUdatabase* m_ffudatabase;
    AuxFanDatabase* m_auxfandatabase;
    OCUdatabase* m_ocudatabase;

    RemoteController* m_remotecontroller;

    QTimer m_timer;
    QTimer m_manualSpeedUpdateTimer;

    int m_speed;

private slots:
    void slot_timer_fired();
    void slot_manualSpeedUpdateTimer_fired();

    void slot_button_operation_clicked();
    void slot_button_error_clicked();
    void slot_button_speed_0_clicked();
    void slot_button_speed_50_clicked();
    void slot_button_speed_100_clicked();

    void slot_remoteControlActivated();
    void slot_remoteControlDeactivated();
    void slot_remoteControlConnected();
    void slot_remoteControlDisconnected();

    void slot_shutdownNOW();
    void slot_mainsPowerRestored();

    void slot_newError();
    void slot_allErrorsQuit();
    void slot_allErrorsGone();
};

#endif // MAINCONTROLLER_H
