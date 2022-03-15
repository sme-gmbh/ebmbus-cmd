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

#ifndef REMOTECLIENTHANDLER_H
#define REMOTECLIENTHANDLER_H

#include <QObject>
#include <QtNetwork>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMap>
#include <QByteArray>
#include <QRegExp>
#include <QHostInfo>

#include "ffudatabase.h"
#include "auxfandatabase.h"
#include "ocudatabase.h"
#include "loghandler.h"

class RemoteClientHandler : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClientHandler(QObject *parent, QTcpSocket* socket, FFUdatabase* ffuDB, AuxFanDatabase* auxFanDB, OCUdatabase *ocuDB, Loghandler *loghandler);

private:
    QTcpSocket* socket;
    FFUdatabase* m_ffuDB;
    AuxFanDatabase* m_auxFanDB;
    OCUdatabase* m_ocuDB;
    Loghandler* m_loghandler;
    bool m_livemode;

signals:
    void signal_broadcast(QByteArray data);
    void signal_connectionClosed(QTcpSocket* socket, RemoteClientHandler* remoteClientHandler);

    void signal_buttonSimulated_operation_clicked();
    void signal_buttonSimulated_error_clicked();
    void signal_buttonSimulated_speed_0_clicked();
    void signal_buttonSimulated_speed_50_clicked();
    void signal_buttonSimulated_speed_100_clicked();

public slots:

private slots:
    void slot_read_ready();
    void slot_disconnected();
    void slot_DCIaddressingFinished(int busID);
    void slot_DCIaddressingGotSerialNumber(int busID, quint8 unit, quint8 fanAddress, quint8 fanGroup, quint32 serialNumber);
    void slot_FFUactualDataHasChanged(int id);
    void slot_AuxFanActualDataHasChanged(int id);
    void slot_OCUfanActualDataHasChanged(int id);
};

#endif // REMOTECLIENTHANDLER_H
