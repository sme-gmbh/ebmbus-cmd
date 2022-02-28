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

#ifndef OCUMODBUSSYSTEM_H
#define OCUMODBUSSYSTEM_H

#include <QObject>
#include <QThread>
#include <QSettings>
#include "loghandler.h"
#include "ocumodbus.h"

class OcuModbusSystem : public QObject
{
    Q_OBJECT
public:
    explicit OcuModbusSystem(QObject *parent, Loghandler* loghandler);
    ~OcuModbusSystem();

    QList<OcuModbus*> *ocuModbuslist();

    OcuModbus* getBusByID(int busID);

    quint64 readHoldingRegister(int busID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg);
    quint64 writeHoldingRegister(int busID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg, quint16 rawdata);
    quint64 readInputRegister(int busID, quint16 adr, OcuModbus::OcuModbusInputRegister reg);


private:
    Loghandler* m_loghandler;
    QList<OcuModbus*> m_ocuModbuslist;
    QThread m_workerThread;

    quint64 getNewTelegramID();

signals:

    // Incoming signals from bus, routed to host
    void signal_transactionLost(quint64 telegramID);
    void signal_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg, quint16 rawdata);
    void signal_receivedInputRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusInputRegister reg, quint16 rawdata);
    void signal_wroteHoldingRegisterData(quint64 telegramID);


    // Outgoing signals to bus
    void signal_writeHoldingRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg, quint16 rawdata);
    void signal_readHoldingRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg);
    void signal_readInputRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusInputRegister reg);


public slots:

private slots:

};

#endif // OCUMODBUSSYSTEM_H
