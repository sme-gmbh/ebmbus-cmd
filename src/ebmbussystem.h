#ifndef EBMBUSSYSTEM_H
#define EBMBUSSYSTEM_H

#include <QObject>
#include <QList>
#include <QMap>
#include <libebmbus/ebmbus.h>
#include "revpidio.h"
#include "daisychaininterface.h"

class EbmBusSystem : public QObject
{
    Q_OBJECT
public:
    explicit EbmBusSystem(QObject *parent, RevPiDIO* io);

    QList<EbmBus*> *ebmbuslist();

    EbmBus* getBusByID(int busID);

    QString broadcast(int busID, QMap<QString,QString> dataMap);
    void broadcastSpeed(quint8 speed);

private:
    QList<EbmBus*> m_ebmbuslist;
    QList<DaisyChainInterface*> m_dcilist;
    RevPiDIO* m_io;

signals:

public slots:

private slots:
    void slot_showResponseRaw(quint64 telegramID, quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data);
};

#endif // EBMBUSSYSTEM_H
