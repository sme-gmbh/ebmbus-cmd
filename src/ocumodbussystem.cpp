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

    qRegisterMetaType<OcuModbus::OcuModbusHoldingRegister>("OcuModbus::OcuModbusHoldingRegister");
    qRegisterMetaType<OcuModbus::OcuModbusInputRegister>("OcuModbus::OcuModbusInputRegister");

    QSettings settings("/etc/openffucontrol/ebmbus-cmd/ebmbus-cmd.ini", QSettings::IniFormat);
    settings.beginGroup("interfacesOcuModBus");

    QStringList interfaceKeyList = settings.childKeys();

    foreach(QString interfacesKey, interfaceKeyList)
    {
        if (!interfacesKey.startsWith("ocumodbus"))
            continue;
        QString interfacesString = settings.value(interfacesKey).toString();
        QStringList interfaces = interfacesString.split(",", QString::SkipEmptyParts);

        if (interfaces.length() == 1)       // Non redundant bus
        {
            QString interface_0 = interfaces.at(0);

            OcuModbus* newOcuModbus = new OcuModbus(nullptr, QString("/dev/").append(interface_0));    // parent must be 0 in order to be moved to workerThread later
            m_ocuModbuslist.append(newOcuModbus);
            newOcuModbus->moveToThread(&m_workerThread);
            m_workerThread.start();
            connect(&m_workerThread, &QThread::finished, newOcuModbus, &QObject::deleteLater);

            connect(newOcuModbus, SIGNAL(signal_newEntry(LogEntry::LoggingCategory,QString,QString)), m_loghandler, SLOT(slot_newEntry(LogEntry::LoggingCategory,QString,QString)));
            connect(newOcuModbus, SIGNAL(signal_entryGone(LogEntry::LoggingCategory,QString,QString)), m_loghandler, SLOT(slot_entryGone(LogEntry::LoggingCategory,QString,QString)));
            m_ocuModbuslist.append(newOcuModbus);

            // Routing of calls to bus
            connect(this, &OcuModbusSystem::signal_readHoldingRegisterData, newOcuModbus, &OcuModbus::slot_readHoldingRegisterData);
            connect(this, &OcuModbusSystem::signal_readInputRegisterData, newOcuModbus, &OcuModbus::slot_readInputRegisterData);
            connect(this, &OcuModbusSystem::signal_writeHoldingRegisterData, newOcuModbus, &OcuModbus::slot_writeHoldingRegisterData);

            // Routing of bus results to master
            connect(newOcuModbus, &OcuModbus::signal_transactionLost, this, &OcuModbusSystem::signal_transactionLost);
            connect(newOcuModbus, &OcuModbus::signal_receivedHoldingRegisterData, this, &OcuModbusSystem::signal_receivedHoldingRegisterData);
            connect(newOcuModbus, &OcuModbus::signal_receivedInputRegisterData, this, &OcuModbusSystem::signal_receivedInputRegisterData);
            connect(newOcuModbus, &OcuModbus::signal_wroteHoldingRegisterData, this, &OcuModbusSystem::signal_wroteHoldingRegisterData);

            if (!newOcuModbus->open())
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
    m_workerThread.quit();
    m_workerThread.wait();
}

QList<OcuModbus *> *OcuModbusSystem::ocuModbuslist()
{
    return (&m_ocuModbuslist);
}

OcuModbus *OcuModbusSystem::getBusByID(int busID)
{
    if (m_ocuModbuslist.length() <= busID)
        return nullptr; // Bus id not available

    OcuModbus* bus = m_ocuModbuslist.at(busID);

    return bus;
}

quint64 OcuModbusSystem::readHoldingRegister(int busID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg)
{
    Q_UNUSED(busID)
    quint64 telegramID = getNewTelegramID();
    emit signal_readHoldingRegisterData(telegramID, adr, reg);
    return telegramID;
}

quint64 OcuModbusSystem::writeHoldingRegister(int busID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg, quint16 rawdata)
{
    Q_UNUSED(busID)
    quint64 telegramID = getNewTelegramID();
    emit signal_writeHoldingRegisterData(telegramID, adr, reg, rawdata);
    return telegramID;
}

quint64 OcuModbusSystem::readInputRegister(int busID, quint16 adr, OcuModbus::OcuModbusInputRegister reg)
{
    Q_UNUSED(busID)
    quint64 telegramID = getNewTelegramID();
    emit signal_readInputRegisterData(telegramID, adr, reg);
    return telegramID;
}


quint64 OcuModbusSystem::getNewTelegramID()
{
    static quint64 id = 1;  // Start counting telegram id with 1. 0 is reserved for error
    id++;

    return id;
}

