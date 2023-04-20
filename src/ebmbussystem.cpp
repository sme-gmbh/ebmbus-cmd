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

    QSettings settings("/etc/openffucontrol/ebmbus-cmd/ebmbus-cmd.ini", QSettings::IniFormat);
    settings.beginGroup("configEbmBus");
    int telegramRepeatCount = settings.value("telegramRepeatCount", 2).toInt();
    int requestTimeout = settings.value("requestTimeout", 300).toInt();
    settings.endGroup();
    settings.beginGroup("interfacesEbmBus");

    QStringList interfaceKeyList = settings.childKeys();

    foreach(QString interfacesKey, interfaceKeyList)
    {
        QString interface_startOfLoop;
        QString interface_endOfLoop;

        if (!interfacesKey.startsWith("ebmbus"))
            continue;
        QString interfacesString = settings.value(interfacesKey).toString();
        QStringList interfaces = interfacesString.split(",", QString::SkipEmptyParts);

        int i;

        if (interfaces.length() == 1)       // Non redundant bus
        {
            bool ok;
            QString interface_0 = interfaces.at(0);
            i = interface_0.right(1).toInt(&ok);
            if (!ok)
                continue;

            interface_startOfLoop = QString("/dev/").append(interface_0);
        }
        else if (interfaces.length() == 2)  // Redundant bus
        {
            bool ok;
            QString interface_0 = interfaces.at(0);
            i = interface_0.right(1).toInt(&ok);
            if (!ok)
                continue;

            QString interface_1 = interfaces.at(1);

            interface_startOfLoop = QString("/dev/").append(interface_0);
            interface_endOfLoop = QString("/dev/").append(interface_1);
        }
        else
            continue;


        // Now create bus system **********************************************************************************************
        EbmBus* newEbmBus = new EbmBus(this, interface_startOfLoop, interface_endOfLoop);
        newEbmBus->setRequestTimeout(requestTimeout);
        newEbmBus->setTelegramRepeatCount(telegramRepeatCount);
        m_ebmbuslist.append(newEbmBus);

        DaisyChainInterface* newDCI = new DaisyChainInterface(this, m_io, i, i);
        m_dcilist.append(newDCI);

        connect(newEbmBus, SIGNAL(signal_setDCIoutput(bool)), newDCI, SLOT(slot_setDCIoutput(bool)));
        connect(newDCI, SIGNAL(signal_DCIloopResponse(bool)), newEbmBus, SLOT(slot_DCIloopResponse(bool)));

        connect(newEbmBus, SIGNAL(signal_responseRaw(quint64,quint8,quint8,quint8,QByteArray)), this, SLOT(slot_showResponseRaw(quint64,quint8,quint8,quint8,QByteArray)));
        connect(newEbmBus, SIGNAL(signal_transactionLost(quint64)), this, SLOT(slot_transactionLost(quint64)));
        connect(newEbmBus, SIGNAL(signal_transactionFinished()), this, SLOT(slot_transactionFinished()));

        if (!newEbmBus->open())
            fprintf(stderr, "EbmBusSystem::EbmBusSystem(): Unable to open serial line %s or %s!\n", interface_startOfLoop.toUtf8().data(), interface_endOfLoop.toUtf8().data());
        else
            fprintf(stderr, "EbmBusSystem::EbmBusSystem(): Activated on %s!\n", interface_startOfLoop.toUtf8().data()); // Tbd.: Write log if redundancy is active
        fflush(stderr);
    }
}

QList<EbmBus*>* EbmBusSystem::ebmbuslist()
{
    return (&m_ebmbuslist);
}

EbmBus *EbmBusSystem::getBusByID(int busID)
{
    if (m_ebmbuslist.length() <= busID)
        return nullptr; // Bus id not available

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
#ifdef QT_DEBUG
    printf("ID: %llu PRE: %02X  commandAndFanaddress: %02X  fanGroup: %02X  data: ", telegramID, preamble, commandAndFanaddress, fanGroup);
    foreach (quint8 byte, data)
    {
        printf("%02X ", byte);
    }
    printf("\n");
    fflush(stdout);
#else
    Q_UNUSED(telegramID)
    Q_UNUSED(preamble)
    Q_UNUSED(commandAndFanaddress)
    Q_UNUSED(fanGroup)
    Q_UNUSED(data)
#endif
}

void EbmBusSystem::slot_transactionLost(quint64 telegramID)
{
#ifdef QT_DEBUG
    printf("ID: %llu Transaction lost.\n", telegramID);
    fflush(stdout);
#else
    Q_UNUSED(telegramID)
#endif
}

void EbmBusSystem::slot_transactionFinished()
{
#ifdef QT_DEBUG
    printf("Transaction finished.\n");
    fflush(stdout);
#endif
}
