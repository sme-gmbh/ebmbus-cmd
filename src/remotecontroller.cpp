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

#include "remotecontroller.h"

RemoteController::RemoteController(QObject *parent, FFUdatabase *ffuDB, AuxFanDatabase *aufFanDB, Loghandler *loghandler) : QObject(parent)
{
#ifdef QT_DEBUG
    printf(stdout, "Server started\n");
#endif

    m_ffuDB = ffuDB;
    m_auxFanDB = aufFanDB;
    m_loghandler = loghandler;
    m_activated = true;
    m_noConnection = true;

    connect(&m_timer_connectionTimeout, SIGNAL(timeout()), this, SLOT(slot_connectionTimeout()));
    connect(this, SIGNAL(signal_connected()), &m_timer_connectionTimeout, SLOT(stop()));
    m_timer_connectionTimeout.setSingleShot(true);
    m_timer_connectionTimeout.start(30000); // 30 Sec.

    connect(&m_server, SIGNAL(newConnection()),  this, SLOT(slot_new_connection()));
    m_server.listen(QHostAddress::LocalHost, 16001);    // Restrict to localhost (ssh tunnel endpoint)
}

RemoteController::~RemoteController()
{

}

bool RemoteController::isConnected()
{
    return (!m_noConnection);
}

bool RemoteController::isActive()
{
    return (m_activated && !m_noConnection);
}

bool RemoteController::isEnabled()
{
    return m_activated;
}

void RemoteController::slot_activate()
{
    if (m_noConnection)
        m_loghandler->slot_newEntry(LogEntry::Error, "Remotecontroller", "No connection to server.");

    m_activated = true;
    slot_broadcast(QString("Status[RemoteController]: Activated").toUtf8());
    m_loghandler->slot_entryGone(LogEntry::Info, "Remotecontroller", "Deactivated - Running in manual mode.");
    emit signal_activated();
}

void RemoteController::slot_deactivate()
{
    m_activated = false;
    slot_broadcast(QString("Status[RemoteController]: Deactivated").toUtf8());
    m_loghandler->slot_newEntry(LogEntry::Info, "Remotecontroller", "Deactivated - Running in manual mode.");
    emit signal_deactivated();
}

void RemoteController::slot_new_connection()
{
    QTcpSocket* newSocket = m_server.nextPendingConnection();
    this->m_socket_list.append(newSocket);

    RemoteClientHandler* remoteClientHandler = new RemoteClientHandler(this, newSocket, m_ffuDB, m_auxFanDB, m_loghandler);
    connect(remoteClientHandler, SIGNAL(signal_broadcast(QByteArray)),
            this, SLOT(slot_broadcast(QByteArray)));
    connect(remoteClientHandler, SIGNAL(signal_connectionClosed(QTcpSocket*,RemoteClientHandler*)),
            this, SLOT(slot_connectionClosed(QTcpSocket*,RemoteClientHandler*)));

    connect(remoteClientHandler, SIGNAL(signal_buttonSimulated_operation_clicked()), this, SIGNAL(signal_buttonSimulated_operation_clicked()));
    connect(remoteClientHandler, SIGNAL(signal_buttonSimulated_error_clicked()), this, SIGNAL(signal_buttonSimulated_error_clicked()));
    connect(remoteClientHandler, SIGNAL(signal_buttonSimulated_speed_0_clicked()), this, SIGNAL(signal_buttonSimulated_speed_0_clicked()));
    connect(remoteClientHandler, SIGNAL(signal_buttonSimulated_speed_50_clicked()), this, SIGNAL(signal_buttonSimulated_speed_50_clicked()));
    connect(remoteClientHandler, SIGNAL(signal_buttonSimulated_speed_100_clicked()), this, SIGNAL(signal_buttonSimulated_speed_100_clicked()));

    if (m_noConnection)
    {
        m_noConnection = false;
        m_loghandler->slot_entryGone(LogEntry::Error, "Remotecontroller", "No connection to server.");
        emit signal_connected();
    }
}

void RemoteController::slot_broadcast(QByteArray data)
{
    foreach(QTcpSocket* socket, this->m_socket_list)
    {
        socket->write(data + "\r\n");
    }
}

void RemoteController::slot_connectionClosed(QTcpSocket *socket, RemoteClientHandler *remoteClientHandler)
{
    this->m_socket_list.removeOne(socket);
    delete remoteClientHandler;
#ifdef QT_DEBUG
    printf (stdout, "ClientHandler deleted\r\n");
#endif

    if (m_socket_list.isEmpty())
    {
        m_noConnection = true;
        m_loghandler->slot_newEntry(LogEntry::Error, "Remotecontroller", "No connection to server.");
        emit signal_disconnected();
    }
}

void RemoteController::slot_connectionTimeout()
{
    m_loghandler->slot_newEntry(LogEntry::Error, "Remotecontroller", "No connection to server.");
}
