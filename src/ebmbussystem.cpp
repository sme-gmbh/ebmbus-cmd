#include "ebmbussystem.h"

EbmBusSystem::EbmBusSystem(QObject *parent, RevPiDIO *io) : QObject(parent)
{
    m_io = io;

    for (int i=0; i<=3; i++)
    {
        EbmBus* newEbmBus = new EbmBus(this, QString("/dev/ttyUSB").append(QString().setNum(i)));
        m_ebmbuslist.append(newEbmBus);

        DaisyChainInterface* newDCI = new DaisyChainInterface(this, m_io, i, i);
        m_dcilist.append(newDCI);

        connect(newEbmBus, SIGNAL(signal_setDCIoutput(bool)), newDCI, SLOT(slot_setDCIoutput(bool)));
        connect(newDCI, SIGNAL(signal_DCIloopResponse(bool)), newEbmBus, SLOT(slot_DCIloopResponse(bool)));

        connect(newEbmBus, SIGNAL(signal_responseRaw(quint64,quint8,quint8,quint8,QByteArray)), this, SLOT(slot_showResponseRaw(quint64,quint8,quint8,quint8,QByteArray)));

        newEbmBus->open();
    }
}

QList<EbmBus*>* EbmBusSystem::ebmbuslist()
{
    return (&m_ebmbuslist);
}

EbmBus *EbmBusSystem::getBusByID(int busID)
{
    if (m_ebmbuslist.length() <= busID)
        return NULL; // Bus id not available

    EbmBus* bus = m_ebmbuslist.at(busID);

    return bus;
}

QString EbmBusSystem::broadcast(int busID, QMap<QString, QString> dataMap)
{
    if (busID >= m_ebmbuslist.count())
        return "Warning[EbmBusSystem]: busID " + QString().setNum(busID) + " invalid";

    EbmBus* ebmBus = m_ebmbuslist.at(busID);
    QString response;

    foreach (QString key, dataMap.keys()) {
        if (key == "rawspeed")
        {
            ebmBus->setSpeedSetpoint(0, 0, dataMap.value("rawspeed").toInt());
            response.append("OK[EbmBusSystem]: Broadcasting speed at bus "+ QString().setNum(busID) + ".\r\n");
        }
    }

    return response;
}

void EbmBusSystem::broadcastSpeed(quint8 speed)
{
    foreach (EbmBus* ebmbus, m_ebmbuslist)
    {
        ebmbus->setSpeedSetpoint(0, 0, speed);
    }
}

// This slot is called if one of the ebmbus interfaces received a telegram as response from an FFU
void EbmBusSystem::slot_showResponseRaw(quint64 telegramID, quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data)
{
    printf("ID: %llu PRE: %02X  commandAndFanaddress: %02X  fanGroup: %02X  data: ", telegramID, preamble, commandAndFanaddress, fanGroup);
    foreach (quint8 byte, data)
    {
        printf("%02X ", byte);
    }
    printf("\n");
}
