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
#include "loghandler.h"

class RemoteController : public QObject
{
    Q_OBJECT
public:
    explicit RemoteController(QObject *parent, FFUdatabase* ffuDB, Loghandler* loghandler);
    ~RemoteController();

    bool isConnected(); // Returns true if at least one server is connected
    bool isActive();    // Returns true if remote controller is supposed to control ffus remotely

private:
    QTcpServer m_server;
    QList<QTcpSocket*> m_socket_list;
    FFUdatabase* m_ffuDB;
    Loghandler* m_loghandler;
    bool m_activated; // True if remote controller is supposed to do remote controlling actions
    bool m_noConnection;  // True if no server is connected

signals:
    void signal_activated();
    void signal_deactivated();
    void signal_connected();
    void signal_disconnected();

public slots:
    void slot_activate();
    void slot_deactivate();

private slots:
    void slot_new_connection();
    void slot_broadcast(QByteArray data);
    void slot_connectionClosed(QTcpSocket* socket, RemoteClientHandler* remoteClientHandler);
};

#endif // REMOTECONTROLLER_H
