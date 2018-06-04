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

class RemoteClientHandler : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClientHandler(QObject *parent, QTcpSocket* socket, FFUdatabase* ffuDB);

private:
    QTcpSocket* socket;
    FFUdatabase* m_ffuDB;
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
};

#endif // REMOTECLIENTHANDLER_H
