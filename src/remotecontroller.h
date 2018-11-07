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

#ifndef REMOTECONTROLLER_H
#define REMOTECONTROLLER_H

#include <QObject>
#include <QtNetwork>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <iostream>
#include "remoteclienthandler.h"
#include "ffudatabase.h"
#include "auxfandatabase.h"
#include "loghandler.h"

class RemoteController : public QObject
{
    Q_OBJECT
public:
    explicit RemoteController(QObject *parent, FFUdatabase* ffuDB, AuxFanDatabase* aufFanDB, Loghandler* loghandler);
    ~RemoteController();

    bool isConnected(); // Returns true if at least one server is connected
    bool isActive();    // Returns true if remote controller is supposed to control ffus remotely and remote connecton is established
    bool isEnabled();   // Returns true if remote controller is supposed to control ffus remotely

private:
    QTcpServer m_server;
    QList<QTcpSocket*> m_socket_list;
    FFUdatabase* m_ffuDB;
    AuxFanDatabase* m_auxFanDB;
    Loghandler* m_loghandler;
    bool m_activated; // True if remote controller is supposed to do remote controlling actions
    bool m_noConnection;  // True if no server is connected
    QTimer m_timer_connectionTimeout;   // Server should connect within this time, otherwise signal error

signals:
    void signal_activated();
    void signal_deactivated();
    void signal_connected();
    void signal_disconnected();

    void signal_buttonSimulated_operation_clicked();
    void signal_buttonSimulated_error_clicked();
    void signal_buttonSimulated_speed_0_clicked();
    void signal_buttonSimulated_speed_50_clicked();
    void signal_buttonSimulated_speed_100_clicked();

public slots:
    void slot_activate();
    void slot_deactivate();

private slots:
    void slot_new_connection();
    void slot_broadcast(QByteArray data);
    void slot_connectionClosed(QTcpSocket* socket, RemoteClientHandler* remoteClientHandler);
    void slot_connectionTimeout();
};

#endif // REMOTECONTROLLER_H
