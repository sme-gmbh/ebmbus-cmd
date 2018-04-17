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

#include "ffudatabase.h"

class RemoteClientHandler : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClientHandler(QObject *parent, QTcpSocket* socket, FFUdatabase* ffuDB);

private:
    QTcpSocket* socket;
    FFUdatabase* m_ffuDB;

signals:
    void signal_broadcast(QByteArray data);
    void signal_connectionClosed(QTcpSocket* socket, RemoteClientHandler* remoteClientHandler);

public slots:

private slots:
    void slot_read_ready();
    void slot_disconnected();
};

#endif // REMOTECLIENTHANDLER_H
