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
    QString directory = "/var/openffucontrol/ocufans/";
    QDirIterator iterator(directory, QStringList() << "*.csv", QDir::Files, QDirIterator::NoIteratorFlags);

    QStringList filepaths;

    while(iterator.hasNext())
    {
        filepaths.append(iterator.next());
    }

    filepaths.sort();

    foreach(QString filepath, filepaths)
    {
        OCUfan* newOCUfan = new OCUfan(this, m_ocuModbusSystem, m_loghandler);
        newOCUfan->load(filepath);
        newOCUfan->setFiledirectory(directory);
        connect(newOCUfan, &OCUfan::signal_FanActualDataHasChanged, this, &OCUdatabase::signal_FanActualDataHasChanged);
        m_ocufans.append(newOCUfan);
    }
}

void OCUdatabase::saveToHdd()
{
    QString path = "/var/openffucontrol/ocufans/";

    foreach (OCUfan* ocufan, m_ocufans)
    {
        ocufan->setFiledirectory(path);
        ocufan->save();
    }
}

QString OCUdatabase::addOCUfan(int id, int busID, int ocuModbusAddress, int fanAddress)
{
    OCUfan* newOCUfan = new OCUfan(this, m_ocuModbusSystem, m_loghandler);
    newOCUfan->setFiledirectory("/var/openffucontrol/ocufans/");
    newOCUfan->setAutoSave(false);
    newOCUfan->setId(id);
    newOCUfan->setBusID(busID);
    newOCUfan->setFanAddress(fanAddress);       // Number of fan on that particular OCU
    newOCUfan->setFanGroup(ocuModbusAddress);   // Busaddress of the OCU on the ModBus
    newOCUfan->setAutoSave(true);
    newOCUfan->save();
    connect(newOCUfan, &OCUfan::signal_FanActualDataHasChanged, this, &OCUdatabase::signal_FanActualDataHasChanged);
    m_ocufans.append(newOCUfan);

    return "OK[OCUdatabase]: Added ID " + QString().setNum(id);
}

QString OCUdatabase::deleteOCUfan(int id)
{
    OCUfan* ocufan = getOCUfansByID(id);
    if (ocufan == nullptr)
        return "Warning[OCUdatabase]: ID " + QString().setNum(id) + " not found.";

    bool ok = m_ocufans.removeOne(ocufan);
    if (ok)
    {
        disconnect(ocufan, &OCUfan::signal_FanActualDataHasChanged, this, &OCUdatabase::signal_FanActualDataHasChanged);
        ocufan->deleteFromHdd();
        ocufan->deleteAllErrors();
        delete ocufan;
        return "OK[OCUdatabase]: Removed ID " + QString().setNum(id);
    }
    return "Warning[OCUdatabase]: Unable to remove ID " + QString().setNum(id) + " from db.";
}

QList<OCUfan *> OCUdatabase::getOCUfans(int busNr)
{
    QList<OCUfan *> ocufanList;
    if (busNr == -1)
        return m_ocufans;
    else
    {
        foreach (OCUfan* ocufan, m_ocufans) {
            if (ocufan->getBusID() == busNr)
                ocufanList.append(ocufan);
        }
    }

    return ocufanList;
}

OCUfan *OCUdatabase::getOCUfansByID(int id)
{
    foreach (OCUfan* ocufan, m_ocufans)
    {
        if (ocufan->getId() == id)
            return ocufan;
    }
    return nullptr;    // Not found
}

OCUfan *OCUdatabase::getOCUfanByTelegramID(quint64 telegramID)
{
    foreach (OCUfan* ocufan, m_ocufans) {
        bool found = ocufan->isThisYourTelegram(telegramID);
        if (found)
        {
            return ocufan;
        }
    }

    return nullptr;    // TransactionID not initiated by ffu requests, so it came frome somebody else
}

QString OCUdatabase::getOCUfanData(int id, QString key)
{
    OCUfan* ocufan = getOCUfansByID(id);
    if (ocufan == nullptr)
        return "Warning[OCUdatabase]: ID " + QString().setNum(id) + " not found.";

    return ocufan->getData(key);
}

QMap<QString, QString> OCUdatabase::getOCUfanData(int id, QStringList keys)
{
    QMap<QString,QString> response;

    OCUfan* ocufan = getOCUfansByID(id);
    if (ocufan == nullptr)
    {
        return response;
    }

    if (keys.contains("actual"))
    {
        keys.clear();   // Only show actual values, drop all other requests because the answer goes into a special processing later
        keys.append(ocufan->getActualKeys());
        response.insert("actualData", "1"); // And show the recipient that this is actualData
    }

    foreach (QString key, keys)
    {
        response.insert(key, ocufan->getData(key));
    }

    return response;
}

QString OCUdatabase::setOCUfanData(int id, QString key, QString value)
{
    OCUfan* ocufan = getOCUfansByID(id);
    if (ocufan == nullptr)
        return "Warning[OCUdatabase]: ID " + QString().setNum(id) + " not found.";

    ocufan->setData(key, value);
    return "OK[OCUdatabase]: Setting " + key + " to " + value;
}

QString OCUdatabase::setOCUfanData(int id, QMap<QString, QString> dataMap)
{
    OCUfan* ocufan = getOCUfansByID(id);
    if (ocufan == nullptr)
        return "Warning[OCUdatabase]: ID " + QString().setNum(id) + " not found.";

    QString dataString;

    foreach(QString key, dataMap.keys())
    {
        QString value = dataMap.value(key);
        ocufan->setData(key, value);

        dataString.append(" " + key + ":" + value);
    }

    return "OK[OCUdatabase]: Setting data:" + dataString;
}

void OCUdatabase::slot_remoteControlActivated()
{
    foreach (OCUfan* ocufan, m_ocufans) {
        ocufan->setRemoteControlled(true);
    }
}

void OCUdatabase::slot_remoteControlDeactivated()
{
    foreach (OCUfan* ocufan, m_ocufans) {
        ocufan->setRemoteControlled(false);
    }
}

void OCUdatabase::slot_transactionFinished()
{
    // Do nothing
}

void OCUdatabase::slot_transactionLost(quint64 telegramID)
{
    OCUfan* ocufan = getOCUfanByTelegramID(telegramID);
    if (ocufan == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    ocufan->slot_transactionLost(telegramID);
}

void OCUdatabase::slot_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, quint16 reg, quint16 rawdata)
{
    OCUfan* ocufan = getOCUfanByTelegramID(telegramID);
    if (ocufan == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    ocufan->slot_receivedHoldingRegisterData(telegramID, adr, reg, rawdata);
}

void OCUdatabase::slot_receivedInputRegisterData(quint64 telegramID, quint16 adr, quint16 reg, quint16 rawdata)
{
    OCUfan* ocufan = getOCUfanByTelegramID(telegramID);
    if (ocufan == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    ocufan->slot_receivedInputRegisterData(telegramID, adr, reg, rawdata);
}

void OCUdatabase::slot_wroteHoldingRegisterData(quint64 telegramID)
{
    OCUfan* ocufan = getOCUfanByTelegramID(telegramID);
    if (ocufan == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    ocufan->slot_wroteHoldingRegisterData(telegramID);
}

void OCUdatabase::slot_timer_pollStatus_fired()
{
    static int currentOCUfanId = 0;

    if (m_ocufans.count() > currentOCUfanId)
    {
        OCUfan* ocufan = m_ocufans.at(currentOCUfanId);
        ocufan->requestStatus();
    }

    currentOCUfanId++;
    if (m_ocufans.count() <= currentOCUfanId)
        currentOCUfanId = 0;
}
