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

#include "ocudatabase.h"

OCUdatabase::OCUdatabase(QObject *parent,  OcuModbusSystem *ocuModbusSystem, Loghandler *loghandler) : QObject(parent)
{
    m_ocuModbusSystem = ocuModbusSystem;

    m_ocuModbusList = ocuModbusSystem->ocuModbuslist(); // Try to eliminate this!

    m_loghandler = loghandler;

    // High level bus-system response connections
    connect(m_ocuModbusSystem, &OcuModbusSystem::signal_receivedHoldingRegisterData, this, &OCUdatabase::slot_receivedHoldingRegisterData);
    connect(m_ocuModbusSystem, &OcuModbusSystem::signal_receivedInputRegisterData, this, &OCUdatabase::slot_receivedInputRegisterData);
    connect(m_ocuModbusSystem, &OcuModbusSystem::signal_wroteHoldingRegisterData, this, &OCUdatabase::slot_wroteHoldingRegisterData);
    connect(m_ocuModbusSystem, &OcuModbusSystem::signal_transactionLost, this, &OCUdatabase::slot_transactionLost);

    connect(&m_timer_pollStatus, &QTimer::timeout, this, &OCUdatabase::slot_timer_pollStatus_fired);
    m_timer_pollStatus.setInterval(2000);
    m_timer_pollStatus.start();
}

void OCUdatabase::loadFromHdd()
{
    QString directory = "/var/openffucontrol/ocus/";
    QDirIterator iterator(directory, QStringList() << "*.csv", QDir::Files, QDirIterator::NoIteratorFlags);

    QStringList filepaths;

    while(iterator.hasNext())
    {
        filepaths.append(iterator.next());
    }

    filepaths.sort();

    foreach(QString filepath, filepaths)
    {
        OCU* newOCU = new OCU(this, m_ocuModbusSystem, m_loghandler);
        newOCU->load(filepath);
        newOCU->setFiledirectory(directory);
        connect(newOCU, &OCU::signal_OCUActualDataHasChanged, this, &OCUdatabase::signal_OCUActualDataHasChanged);
        m_ocus.append(newOCU);
    }
}

void OCUdatabase::saveToHdd()
{
    QString path = "/var/openffucontrol/ocus/";

    foreach (OCU* ocu, m_ocus)
    {
        ocu->setFiledirectory(path);
        ocu->save();
    }
}

QString OCUdatabase::addOCU(int id, int busID, int fanAddress)
{
    OCU* newOCU = new OCU(this, m_ocuModbusSystem, m_loghandler);
    newOCU->setFiledirectory("/var/openffucontrol/ocus/");
    newOCU->setAutoSave(false);
    newOCU->setId(id);
    newOCU->setBusID(busID);
    newOCU->setFanAddress(fanAddress);
    newOCU->setAutoSave(true);
    newOCU->save();
    connect(newOCU, &OCU::signal_OCUActualDataHasChanged, this, &OCUdatabase::signal_OCUActualDataHasChanged);
    m_ocus.append(newOCU);

    return "OK[OCUdatabase]: Added OCU ID " + QString().setNum(id);
}

QString OCUdatabase::deleteOCU(int id)
{
    OCU* ocu = getOCUByID(id);
    if (ocu == nullptr)
        return "Warning[OCUdatabase]: ID " + QString().setNum(id) + " not found.";

    bool ok = m_ocus.removeOne(ocu);
    if (ok)
    {
        disconnect(ocu, &OCU::signal_OCUActualDataHasChanged, this, &OCUdatabase::signal_OCUActualDataHasChanged);
        ocu->deleteFromHdd();
        ocu->deleteAllErrors();
        delete ocu;
        return "OK[OCUdatabase]: Removed ID " + QString().setNum(id);
    }
    return "Warning[OCUdatabase]: Unable to remove ID " + QString().setNum(id) + " from db.";
}

QList<OCU *> OCUdatabase::getOCUs(int busNr)
{
    QList<OCU *> ocuList;
    if (busNr == -1)
        return m_ocus;
    else
    {
        foreach (OCU* ocu, m_ocus) {
            if (ocu->getBusID() == busNr)
                ocuList.append(ocu);
        }
    }

    return ocuList;
}

OCU *OCUdatabase::getOCUByID(int id)
{
    foreach (OCU* ocu, m_ocus)
    {
        if (ocu->getId() == id)
            return ocu;
    }
    return nullptr;    // Not found
}

OCU *OCUdatabase::getOCUByTelegramID(quint64 telegramID)
{
    foreach (OCU* ocu, m_ocus) {
        bool found = ocu->isThisYourTelegram(telegramID);
        if (found)
        {
            return ocu;
        }
    }

    return nullptr;    // TransactionID not initiated by ffu requests, so it came frome somebody else
}

QString OCUdatabase::getOCUData(int id, QString key)
{
    OCU* ocu = getOCUByID(id);
    if (ocu == nullptr)
        return "Warning[OCUdatabase]: ID " + QString().setNum(id) + " not found.";

    return ocu->getData(key);
}

QMap<QString, QString> OCUdatabase::getOCUData(int id, QStringList keys)
{
    QMap<QString,QString> response;

    OCU* ocu = getOCUByID(id);
    if (ocu == nullptr)
    {
        return response;
    }

    if (keys.contains("actual"))
    {
        keys.clear();   // Only show actual values, drop all other requests because the answer goes into a special processing later
        keys.append(ocu->getActualKeys());
        response.insert("actualData", "1"); // And show the recipient that this is actualData
    }

    foreach (QString key, keys)
    {
        response.insert(key, ocu->getData(key));
    }

    return response;
}

QString OCUdatabase::setOCUData(int id, QString key, QString value)
{
    OCU* ocu = getOCUByID(id);
    if (ocu == nullptr)
        return "Warning[OCUdatabase]: ID " + QString().setNum(id) + " not found.";

    ocu->setData(key, value);
    return "OK[OCUdatabase]: Setting " + key + " to " + value;
}

QString OCUdatabase::setOCUData(int id, QMap<QString, QString> dataMap)
{
    OCU* ocu = getOCUByID(id);
    if (ocu == nullptr)
        return "Warning[OCUdatabase]: ID " + QString().setNum(id) + " not found.";

    QString dataString;

    foreach(QString key, dataMap.keys())
    {
        QString value = dataMap.value(key);
        ocu->setData(key, value);

        dataString.append(" " + key + ":" + value);
    }

    return "OK[OCUdatabase]: Setting data:" + dataString;
}

void OCUdatabase::slot_remoteControlActivated()
{
    foreach (OCU* ocu, m_ocus) {
        ocu->setRemoteControlled(true);
    }
}

void OCUdatabase::slot_remoteControlDeactivated()
{
    foreach (OCU* ocu, m_ocus) {
        ocu->setRemoteControlled(false);
    }
}

void OCUdatabase::slot_transactionFinished()
{
    // Do nothing
}

void OCUdatabase::slot_transactionLost(quint64 telegramID)
{
    OCU* ocu = getOCUByTelegramID(telegramID);
    if (ocu == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    ocu->slot_transactionLost(telegramID);
}

void OCUdatabase::slot_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg, quint16 rawdata)
{
    OCU* ocu = getOCUByTelegramID(telegramID);
    if (ocu == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    ocu->slot_receivedHoldingRegisterData(telegramID, adr, reg, rawdata);
}

void OCUdatabase::slot_receivedInputRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusInputRegister reg, quint16 rawdata)
{
    OCU* ocu = getOCUByTelegramID(telegramID);
    if (ocu == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    ocu->slot_receivedInputRegisterData(telegramID, adr, reg, rawdata);
}

void OCUdatabase::slot_wroteHoldingRegisterData(quint64 telegramID)
{
    OCU* ocu = getOCUByTelegramID(telegramID);
    if (ocu == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    ocu->slot_wroteHoldingRegisterData(telegramID);
}

void OCUdatabase::slot_timer_pollStatus_fired()
{
    static int currentOCUId = 0;

    if (m_ocus.count() > currentOCUId)
    {
        OCU* ocu = m_ocus.at(currentOCUId);
        ocu->requestStatus();
    }

    currentOCUId++;
    if (m_ocus.count() <= currentOCUId)
        currentOCUId = 0;
}
