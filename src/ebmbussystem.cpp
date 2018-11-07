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

#include "ebmbussystem.h"

EbmBusSystem::EbmBusSystem(QObject *parent, RevPiDIO *io) : QObject(parent)
{
    m_io = io;

    for (int i=0; i<=2; i++)    // tbd.: Get the interfaces from config!
    {
        EbmBus* newEbmBus = new EbmBus(this, QString("/dev/ttyUSB").append(QString().setNum(i)));
        m_ebmbuslist.append(newEbmBus);

        DaisyChainInterface* newDCI = new DaisyChainInterface(this, m_io, i, i);
        m_dcilist.append(newDCI);

        connect(newEbmBus, SIGNAL(signal_setDCIoutput(bool)), newDCI, SLOT(slot_setDCIoutput(bool)));
        connect(newDCI, SIGNAL(signal_DCIloopResponse(bool)), newEbmBus, SLOT(slot_DCIloopResponse(bool)));

        connect(newEbmBus, SIGNAL(signal_responseRaw(quint64,quint8,quint8,quint8,QByteArray)), this, SLOT(slot_showResponseRaw(quint64,quint8,quint8,quint8,QByteArray)));
        connect(newEbmBus, SIGNAL(signal_transactionLost(quint64)), this, SLOT(slot_transactionLost(quint64)));
        connect(newEbmBus, SIGNAL(signal_transactionFinished()), this, SLOT(slot_transactionFinished()));

        if (!newEbmBus->open())
            printf("Unable to open serial line!\n");
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

void EbmBusSystem::broadcastSpeed(quint8 speed, bool disableAutosaveAndAutostart)
{
    foreach (EbmBus* ebmbus, m_ebmbuslist)
    {
        if (disableAutosaveAndAutostart)
        {
            ebmbus->writeEEPROM(0, 0, EbmBusEEPROM::OperationModes_1, 0x03);
            ebmbus->softwareReset(0, 0);
        }
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
    fflush(stdout);
}

void EbmBusSystem::slot_transactionLost(quint64 telegramID)
{
    printf("ID: %llu Transaction lost.\n", telegramID);
    fflush(stdout);
}

void EbmBusSystem::slot_transactionFinished()
{
    printf("Transaction finished.\n");
    fflush(stdout);
}
