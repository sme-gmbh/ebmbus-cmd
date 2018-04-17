#include "remotecontroller.h"

RemoteController::RemoteController(QObject *parent, FFUdatabase *ffuDB) : QObject(parent)
{
#ifdef DEBUG
    printf("Server started\n");
#endif

    m_ffuDB = ffuDB;
    m_activated = false;
    m_noConnection = true;

    connect(&m_server, SIGNAL(newConnection()),  this, SLOT(slot_new_connection()));
    m_server.listen(QHostAddress::AnyIPv4, 16001);    // Todo: Restrict to localhost (ssh tunnel)
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

void RemoteController::slot_activate()
{
    if (m_noConnection)
        return;

    m_activated = true;
    slot_broadcast(QString("Status[RemoteController]: Activated").toUtf8());
    emit signal_activated();
}

void RemoteController::slot_deactivate()
{
    m_activated = false;
    slot_broadcast(QString("Status[RemoteController]: Deactivated").toUtf8());
    emit signal_deactivated();
}

void RemoteController::slot_new_connection()
{
    QTcpSocket* newSocket = m_server.nextPendingConnection();
    this->m_socket_list.append(newSocket);

    RemoteClientHandler* remoteClientHandler = new RemoteClientHandler(this, newSocket, m_ffuDB);
    connect(remoteClientHandler, SIGNAL(signal_broadcast(QByteArray)),
            this, SLOT(slot_broadcast(QByteArray)));
    connect(remoteClientHandler, SIGNAL(signal_connectionClosed(QTcpSocket*,RemoteClientHandler*)),
            this, SLOT(slot_connectionClosed(QTcpSocket*,RemoteClientHandler*)));

    if (m_noConnection)
    {
        m_noConnection = false;
        emit signal_connected();
    }
}

void RemoteController::slot_broadcast(QByteArray data)
{
    foreach(QTcpSocket* socket, this->m_socket_list)
    {
        socket->write(data);
    }
}

void RemoteController::slot_connectionClosed(QTcpSocket *socket, RemoteClientHandler *remoteClientHandler)
{
    this->m_socket_list.removeOne(socket);
    delete remoteClientHandler;
#ifdef DEBUG
    printf ("ClientHandler deleted\r\n");
#endif

    if (m_socket_list.isEmpty())
    {
        m_noConnection = true;
        emit signal_disconnected();
    }
}
