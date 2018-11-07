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

#ifndef UNINTERRUPTIBLEPOWERSUPPLY_H
#define UNINTERRUPTIBLEPOWERSUPPLY_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include "revpidio.h"
#include "loghandler.h"

#ifdef USE_LIBFTDI
#include <ftdi.h>
#elif USE_LIBFTDI1
#include <libftdi1/ftdi.h>
#endif

class UninterruptiblePowerSupply : public QObject
{
    Q_OBJECT
public:
    explicit UninterruptiblePowerSupply(QObject *parent, RevPiDIO *io, int address_mainswitch, Loghandler* loghandler);

    void setShutdownTimeout(int milliseconds);
    void setMainswitchDelay(int milliseconds);
    void setPowerGoodDelay(int milliseconds);

private:
    RevPiDIO* m_io;
    int m_address_mainswitch;
    Loghandler* m_loghandler;
    QTimer m_timer;
    QTimer m_shutdownTimer;
    QTimer m_powerGoodTimer;
    QDateTime m_dateTime_mainSwitchOff;
    QDateTime m_dateTime_powerfail;
    int m_shutdownTimeout;
    int m_mainswitchDelay;
    bool m_mainswitchState;
    bool m_powerstate;
    bool m_old_mainswitchState;
    bool m_mainswitchOffSignaled;

    struct ftdi_context *m_ftdi;
    QByteArray m_upsResponseBuffer;

    QTimer m_ups_communicationWatchdogTimer;
    bool m_ups_communicationOK;

    bool m_ups_bufferReady;     // True if ups could take over load to island operation
    bool m_ups_supplyVoltageOK; // True if input voltage is ok
    bool m_ups_runningInIsland; // True if power is provided by internal energy storage
    bool m_ups_energyStorageHighLevel;  // True if more than 85 % energy level is available in storage

    void connectToUPS();
    void disconnectFromUPS();
    QByteArray readFromUPS();
    void tryToParseUPSresponse(QByteArray* buffer);

signals:
    void signal_ups_communicationHeartbeat();   // Triggers every time the ups sends a valid data string
    void signal_ups_communicationFailure();     // Triggers if ups did not send a valid data string for some reasonable amount of time

    void signal_mainswitchOff();
    void signal_shutdownDueToPowerloss();
    void signal_powerGoodAgain();

    void signal_warning_DCinputVoltageLow();
    void signal_info_DCinputVoltageOK();
    void signal_warning_UPSinIsland();
    void signal_info_UPSonline();
    void signal_alarm_EnergyStorageLow();
    void signal_info_EnergyStorageReady();
    void signal_prewarning_EnergyStorageNotFull();
    void signal_info_EnergyStorageFull();

public slots:
    void slot_startPSUshutdownTimer();

private slots:
    void slot_timer_fired();
    void slot_ups_communicationHeartbeat();
    void slot_ups_communicationFailure();
};

#endif // UNINTERRUPTIBLEPOWERSUPPLY_H
