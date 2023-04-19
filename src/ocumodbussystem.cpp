/**********************************************************************
** ebmbus-cmd - a commandline tool to control ebm papst fans
** Copyright (C) 2022 Smart Micro Engineering GmbH
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

#include "ocumodbussystem.h"

OcuModbusSystem::OcuModbusSystem(QObject *parent, Loghandler *loghandler) : QObject(parent)
{
    m_loghandler = loghandler;

//    qRegisterMetaType<OcuModbus::OcuModbusHoldingRegister>("OcuModbus::OcuModbusHoldingRegister");
//    qRegisterMetaType<OcuModbus::OcuModbusInputRegister>("OcuModbus::OcuModbusInputRegister");

    QSettings settings("/etc/openffucontrol/ebmbus-cmd/ebmbus-cmd.ini", QSettings::IniFormat);
    settings.beginGroup("interfacesOcuModBus");

    QStringList interfaceKeyList = settings.childKeys();

    foreach(QString interfacesKey, interfaceKeyList)
    {
        if (!interfacesKey.startsWith("ocumodbus"))
            continue;
        QString interfacesString = settings.value(interfacesKey).toString();
        QStringList interfaces = interfacesString.split(",", Qt::SkipEmptyParts);

        if (interfaces.length() == 1)       // Non redundant bus
        {
            QString interface_0 = interfaces.at(0);

            ModBus* newOcuModbus = new ModBus(nullptr, QString("/dev/").append(interface_0));    // parent must be 0 in order to be moved to workerThread later
            m_ocuModbuslist.append(newOcuModbus);

            connect(this, &OcuModbusSystem::signal_newEntry, m_loghandler, &Loghandler::slot_newEntry);
            connect(this, &OcuModbusSystem::signal_entryGone, m_loghandler, &Loghandler::slot_entryGone);

            // Routing of bus results to master
            connect(newOcuModbus, &ModBus::signal_transactionFinished, this, &OcuModbusSystem::slot_transactionFinished);
            connect(newOcuModbus, &ModBus::signal_transactionFinished, this, &OcuModbusSystem::signal_transactionFinished);
            connect(newOcuModbus, &ModBus::signal_transactionLost, this, &OcuModbusSystem::slot_transactionLost);
            connect(newOcuModbus, &ModBus::signal_transactionLost, this, &OcuModbusSystem::signal_transactionLost);
            connect(newOcuModbus, &ModBus::signal_responseRaw, this, &OcuModbusSystem::slot_responseRaw);
            connect(newOcuModbus, &ModBus::signal_holdingRegistersRead, this, &OcuModbusSystem::slot_holdingRegistersRead);
            connect(newOcuModbus, &ModBus::signal_inputRegistersRead, this, &OcuModbusSystem::slot_inputRegistersRead);

            newOcuModbus->setDelayTxTimer(200);

            if (!newOcuModbus->open(QSerialPort::Baud115200))
                fprintf(stderr, "OcuModbusSystem::OcuModbusSystem(): Unable to open serial line %s!\n", interface_0.toUtf8().data());
            else
                fprintf(stderr, "OcuModbusSystem::OcuModbusSystem(): Activated on %s!\n", interface_0.toUtf8().data());
            fflush(stderr);
        }
        else if (interfaces.length() == 2)  // Redundant bus
        {
            QString interface_0 = interfaces.at(0);
            QString interface_1 = interfaces.at(1);
            // Not implemented yet
        }
    }
}

OcuModbusSystem::~OcuModbusSystem()
{

}

QList<ModBus *> *OcuModbusSystem::ocuModbuslist()
{
    return (&m_ocuModbuslist);
}

ModBus *OcuModbusSystem::getBusByID(int busID)
{
    if (m_ocuModbuslist.length() <= busID)
        return nullptr; // Bus id not available

    ModBus* bus = m_ocuModbuslist.at(busID);

    return bus;
}

void OcuModbusSystem::slot_responseRaw(quint64 telegramID, quint8 address, quint8 functionCode, QByteArray data)
{
#ifdef QT_DEBUG
    printf("ID: %llu ADR: %02X  FC: %02X data: ", telegramID, address, functionCode);
    foreach (quint8 byte, data)
    {
        printf("%02X ", byte);
    }
    printf("\n");
    fflush(stdout);
#else
    Q_UNUSED(telegramID)
    Q_UNUSED(address)
    Q_UNUSED(functionCode)
    Q_UNUSED(data)
#endif
}

void OcuModbusSystem::slot_transactionLost(quint64 telegramID)
{
    emit signal_newEntry(LogEntry::Info, "OcuModbusSystem", QString("Transaction lost."));

#ifdef QT_DEBUG
    printf("ID: %llu Transaction lost.\n", telegramID);
    fflush(stdout);
#else
    Q_UNUSED(telegramID)
#endif
}

void OcuModbusSystem::slot_transactionFinished()
{
#ifdef QT_DEBUG
    printf("Transaction finished.\n");
    fflush(stdout);
#endif
}

void OcuModbusSystem::slot_holdingRegistersRead(quint64 telegramID, quint8 slaveAddress, quint16 dataStartAddress, QList<quint16> data)
{
    quint16 reg = dataStartAddress;

    foreach(quint16 rawdata, data)
    {
        emit signal_receivedHoldingRegisterData(telegramID, slaveAddress, reg, rawdata);
        reg++;
    }
}

void OcuModbusSystem::slot_inputRegistersRead(quint64 telegramID, quint8 slaveAddress, quint16 dataStartAddress, QList<quint16> data)
{
    quint16 reg = dataStartAddress;

    foreach(quint16 rawdata, data)
    {
        emit signal_receivedInputRegisterData(telegramID, slaveAddress, reg, rawdata);
        reg++;
    }
}

//quint64 OcuModbusSystem::readHoldingRegister(int busID, quint16 adr, quint16 reg)
//{
//    Q_UNUSED(busID)
//    quint64 telegramID = getNewTelegramID();
//    oc
//    emit signal_readHoldingRegisterData(telegramID, adr, reg);
//    return telegramID;
//}

//quint64 OcuModbusSystem::writeHoldingRegister(int busID, quint16 adr, quint16 reg, quint16 rawdata)
//{
//    Q_UNUSED(busID)
//    quint64 telegramID = getNewTelegramID();
//    emit signal_writeHoldingRegisterData(telegramID, adr, reg, rawdata);
//    return telegramID;
//}

//quint64 OcuModbusSystem::readInputRegister(int busID, quint16 adr, quint16 reg)
//{
//    Q_UNUSED(busID)
//    quint64 telegramID = getNewTelegramID();
//    emit signal_readInputRegisterData(telegramID, adr, reg);
//    return telegramID;
//}


//quint64 OcuModbusSystem::getNewTelegramID()
//{
//    static quint64 id = 1;  // Start counting telegram id with 1. 0 is reserved for error
//    id++;

//    return id;
//}

