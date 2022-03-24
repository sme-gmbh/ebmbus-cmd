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

#include "ffudatabase.h"

FFUdatabase::FFUdatabase(QObject *parent, EbmBusSystem *ebmbusSystem, Loghandler *loghandler) : QObject(parent)
{
    m_ebmbusSystem = ebmbusSystem;

    m_ebmbuslist = ebmbusSystem->ebmbuslist();  // Try to eliminate the use of ebmbuslist here later!

    m_loghandler = loghandler;

    foreach (EbmBus* ebmBus, *m_ebmbuslist)
    {
        // Bus management connections
        connect(ebmBus, SIGNAL(signal_DaisyChainAdressingFinished()), this, SLOT(slot_DaisyChainAdressingFinished()));
        connect(ebmBus, SIGNAL(signal_DaisyChainAddressingGotSerialNumber(quint8,quint8,quint8,quint32)), this, SLOT(slot_DaisyChainAddressingGotSerialNumber(quint8,quint8,quint8,quint32)));

        // Low level bus response connections
        connect (ebmBus, SIGNAL(signal_responseRaw(quint64,quint8,quint8,quint8,QByteArray)), this, SLOT(slot_gotResponseRaw(quint64,quint8,quint8,quint8,QByteArray)));

        // High level bus response connections
        connect (ebmBus, SIGNAL(signal_actualSpeed(quint64,quint8,quint8,quint8)), this, SLOT(slot_actualSpeed(quint64,quint8,quint8,quint8)));
        connect (ebmBus, SIGNAL(signal_EEPROMdata(quint64,quint8,quint8,EbmBusEEPROM::EEPROMaddress,quint8)), this, SLOT(slot_EEPROMdata(quint64,quint8,quint8,EbmBusEEPROM::EEPROMaddress,quint8)));
        connect (ebmBus, SIGNAL(signal_EEPROMhasBeenWritten(quint64,quint8,quint8)), this, SLOT(slot_EEPROMhasBeenWritten(quint64,quint8,quint8)));
        connect (ebmBus, SIGNAL(signal_setPointHasBeenSet(quint64,quint8,quint8)), this, SLOT(slot_setPointHasBeenSet(quint64,quint8,quint8)));
        connect (ebmBus, SIGNAL(signal_simpleStatus(quint64,quint8,quint8,QString)), this, SLOT(slot_simpleStatus(quint64,quint8,quint8,QString)));
        connect (ebmBus, SIGNAL(signal_status(quint64,quint8,quint8,quint8,QString,quint8)), this, SLOT(slot_status(quint64,quint8,quint8,quint8,QString,quint8)));
        connect (ebmBus, SIGNAL(signal_transactionFinished()), this, SLOT(slot_transactionFinished()));
        connect (ebmBus, SIGNAL(signal_transactionLost(quint64)), this, SLOT(slot_transactionLost(quint64)));
    }

    // Timer for fast motor speed polling sequence
    m_timer_fastSpeedPolling.setSingleShot(true);
    m_timer_fastSpeedPolling.setInterval(30000);    // 30 Seconds of fast polling

    // Timer for cyclic poll task to get the status of ffus
    connect(&m_timer_pollStatus, SIGNAL(timeout()), this, SLOT(slot_timer_pollStatus_fired()));
    m_timer_pollStatus.setInterval(2000);
    m_timer_pollStatus.start();
}

void FFUdatabase::loadFromHdd()
{
    QString directory = "/var/openffucontrol/ffus/";
    QDirIterator iterator(directory, QStringList() << "*.csv", QDir::Files, QDirIterator::NoIteratorFlags);

    QStringList filepaths;

    while(iterator.hasNext())
    {
        filepaths.append(iterator.next());
    }

    filepaths.sort();

    foreach(QString filepath, filepaths)
    {
        FFU* newFFU = new FFU(this, m_ebmbusSystem, m_loghandler);
        newFFU->load(filepath);
        newFFU->setFiledirectory(directory);
        connect(newFFU, SIGNAL(signal_FFUactualDataHasChanged(int)), this, SIGNAL(signal_FFUactualDataHasChanged(int)));
        m_ffus.append(newFFU);
    }
}

void FFUdatabase::saveToHdd()
{
    QString path = "/var/openffucontrol/ffus/";

    foreach (FFU* ffu, m_ffus)
    {
        ffu->setFiledirectory(path);
        ffu->save();
    }
}

QList<EbmBus *>* FFUdatabase::getBusList()
{
    return m_ebmbuslist;
}

QString FFUdatabase::addFFU(int id, int busID, int unit, int fanAddress, int fanGroup)
{
    FFU* newFFU = new FFU(this, m_ebmbusSystem, m_loghandler);
    newFFU->setFiledirectory("/var/openffucontrol/ffus/");
    newFFU->setAutoSave(false);
    newFFU->setId(id);
    newFFU->setBusID(busID);
    newFFU->setUnit(unit);
    newFFU->setFanAddress(fanAddress);
    newFFU->setFanGroup(fanGroup);
    newFFU->setAutoSave(true);
    newFFU->save();
    connect(newFFU, SIGNAL(signal_FFUactualDataHasChanged(int)), this, SIGNAL(signal_FFUactualDataHasChanged(int)));
    m_ffus.append(newFFU);

    return "OK[FFUdatabase]: Added FFU ID " + QString().setNum(id);
}

QString FFUdatabase::deleteFFU(int id)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == nullptr)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    bool ok = m_ffus.removeOne(ffu);
    if (ok)
    {
        disconnect(ffu, SIGNAL(signal_FFUactualDataHasChanged(int)), this, SIGNAL(signal_FFUactualDataHasChanged(int)));
        ffu->deleteFromHdd();
        ffu->deleteAllErrors();
        delete ffu;
        return "OK[FFUdatabase]: Removed ID " + QString().setNum(id);
    }
    return "Warning[FFUdatabase]: Unable to remove ID " + QString().setNum(id) + " from db.";
}

QList<FFU *> FFUdatabase::getFFUs(int busNr)
{
    QList<FFU *> ffuList;
    if (busNr == -1)
        return m_ffus;
    else
    {
        foreach (FFU* ffu, m_ffus) {
            if (ffu->getBusID() == busNr)
                ffuList.append(ffu);
        }
    }

    return ffuList;
}

FFU *FFUdatabase::getFFUbyID(int id)
{
    foreach (FFU* ffu, m_ffus)
    {
        if (ffu->getId() == id)
            return ffu;
    }
    return nullptr;    // Not found
}

FFU *FFUdatabase::getFFUbyTelegramID(quint64 telegramID)
{
    foreach (FFU* ffu, m_ffus) {
        bool found = ffu->isThisYourTelegram(telegramID);
        if (found)
        {
            return ffu;
        }
    }

    return nullptr;    // TransactionID not initiated by ffu requests, so it came frome somebody else
}

void FFUdatabase::slot_remoteControlActivated()
{
    foreach (FFU* ffu, m_ffus) {
        ffu->setRemoteControlled(true);
    }
}

void FFUdatabase::slot_remoteControlDeactivated()
{
    foreach (FFU* ffu, m_ffus) {
        ffu->setRemoteControlled(false);
    }
}

QString FFUdatabase::getFFUdata(int id, QString key)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == nullptr)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    return ffu->getData(key);
}

QMap<QString,QString> FFUdatabase::getFFUdata(int id, QStringList keys)
{
    QMap<QString,QString> response;

    FFU* ffu = getFFUbyID(id);
    if (ffu == nullptr)
    {
        return response;
    }

    if (keys.contains("actual"))
    {
        keys.clear();   // Only show actual values, drop all other requests because the answer goes into a special processing later
        keys.append(ffu->getActualKeys());
        response.insert("actualData", "1"); // And show the recipient that this is actualData
    }

    foreach (QString key, keys)
    {
        response.insert(key, ffu->getData(key));
    }

    return response;
}

QString FFUdatabase::setFFUdata(int id, QString key, QString value)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == nullptr)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    if ((key == "nSet") || (key == "rawspeed"))
        slot_startFastSpeedPollingSequence();

    ffu->setData(key, value);
    return "OK[FFUdatabase]: Setting " + key + " to " + value;
}

QString FFUdatabase::setFFUdata(int id, QMap<QString, QString> dataMap)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == nullptr)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    if (dataMap.keys().contains("nSet") || dataMap.keys().contains("rawspeed"))
        slot_startFastSpeedPollingSequence();

    QString dataString;

    foreach(QString key, dataMap.keys())
    {
        QString value = dataMap.value(key);
        ffu->setData(key, value);

        dataString.append(" " + key + ":" + value);
    }

    return "OK[FFUdatabase]: Setting data:" + dataString;
}

QString FFUdatabase::startDCIaddressing(int busID, QString startAddress, QString idsString)
{
    if (m_ebmbuslist->count() > busID)
    {
        QList<int> ids;

        QStringList parts = idsString.split(",", QString::SkipEmptyParts);
        if (parts.count() > 1)
        {
            foreach(QString part, parts)
            {
                bool ok;
                int id = part.toInt(&ok);
                if (!ok)
                    continue;
                ids.append(id);
            }
        }
        else if (parts.count() == 1)
        {
            // Fill up 254 more units in list, just increment their number
            int id = parts.at(0).toInt();
            for (int i = 0; i < 255; i++)
            {
                ids.append(id + i);
            }
        }

        // Now remember which ids belong to which bus segment
        m_unitIdsPerBus.remove(busID);
        m_unitIdsPerBus.insert(busID, ids);

        m_timer_pollStatus.stop();  // Stop polling units on bus while addressing is in progress
        m_ebmbuslist->at(busID)->clearTelegramQueue(false); // Drop all pending packets from standard priority queue
        m_ebmbuslist->at(busID)->startDaisyChainAddressing();
    }
    return "OK[FFUdatabase]: Starting DCI addressing at bus " + QString().setNum(busID) + ". Ignoring startAddress at the moment. Will be fixed later.";
}

QString FFUdatabase::broadcast(int busID, QMap<QString, QString> dataMap)
{
    if (dataMap.keys().contains("nSet") || dataMap.keys().contains("rawspeed"))
        slot_startFastSpeedPollingSequence();
    return m_ebmbusSystem->broadcast(busID, dataMap);
}

void FFUdatabase::slot_DaisyChainAdressingFinished()
{
    int i = 0;
    QObject* obj = sender();    // Look up who sent that signal
    foreach (EbmBus* ebmBus, *m_ebmbuslist) {
        if (ebmBus == qobject_cast<EbmBus*>(obj))
        {
            emit signal_DCIaddressingFinished(i);   // And now globally tell everybody which bus finished addressing
            m_unitIdsPerBus.remove(i);              // And clean up temporary storage for new unit ids
        }
        i++;
    }
    m_timer_pollStatus.start();                     // Start polling again
}

void FFUdatabase::slot_DaisyChainAddressingGotSerialNumber(quint8 unit, quint8 fanAddress, quint8 fanGroup, quint32 serialNumber)
{
    int i = 0;
    QObject* obj = sender();    // Look up who sent that signal
    foreach (EbmBus* ebmBus, *m_ebmbuslist) {
        if (ebmBus == qobject_cast<EbmBus*>(obj))
        {
            // Try to lookup unit id from given list
            int id = -1;
            QList<int> idList = m_unitIdsPerBus.value(i);
            if (!idList.isEmpty())
            {
                id = idList.takeFirst();
                m_unitIdsPerBus.insert(i, idList);
                this->addFFU(id, i, unit, fanAddress, fanGroup);

            }

            emit signal_DCIaddressingGotSerialNumber(i, unit, fanAddress, fanGroup, serialNumber);   // And now globally tell everybody the serialnumber
        }
        i++;
    }
}

void FFUdatabase::slot_gotResponseRaw(quint64 telegramID, quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data)
{
    Q_UNUSED(telegramID)
    Q_UNUSED(preamble)
    Q_UNUSED(commandAndFanaddress)
    Q_UNUSED(fanGroup)
    Q_UNUSED(data)

    // Dont care about raw responses at the moment
    // These will be parsed by libebmbus anyway and sent to the right highlevel slot
}

void FFUdatabase::slot_transactionFinished()
{

}

void FFUdatabase::slot_transactionLost(quint64 telegramID)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    ffu->slot_transactionLost(telegramID);
}

void FFUdatabase::slot_simpleStatus(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, QString status)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        m_loghandler->slot_newEntry(LogEntry::Error, "FFUdatabase::slot_simpleStatus", "Telegram id mismatch.");
        return;
    }
    ffu->slot_simpleStatus(telegramID, fanAddress, fanGroup, status);
}

void FFUdatabase::slot_status(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 statusAddress, QString status, quint8 rawValue)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        m_loghandler->slot_newEntry(LogEntry::Error, "FFUdatabase::slot_status", "Telegram id mismatch.");
        return;
    }
    ffu->slot_status(telegramID, fanAddress, fanGroup, statusAddress, status, rawValue);
}

void FFUdatabase::slot_actualSpeed(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 actualRawSpeed)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        m_loghandler->slot_newEntry(LogEntry::Error, "FFUdatabase::slot_actualSpeed", "Telegram id mismatch.");
        return;
    }
    ffu->slot_actualSpeed(telegramID, fanAddress, fanGroup, actualRawSpeed);
}

void FFUdatabase::slot_setPointHasBeenSet(quint64 telegramID, quint8 fanAddress, quint8 fanGroup)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        m_loghandler->slot_newEntry(LogEntry::Error, "FFUdatabase::slot_setPointHasBeenSet", "Telegram id mismatch.");
        return;
    }
    ffu->slot_setPointHasBeenSet(telegramID, fanAddress, fanGroup);
}

void FFUdatabase::slot_EEPROMhasBeenWritten(quint64 telegramID, quint8 fanAddress, quint8 fanGroup)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        m_loghandler->slot_newEntry(LogEntry::Error, "FFUdatabase::slot_EEPROMhasBeenWritten", "Telegram id mismatch.");
        return;
    }
    ffu->slot_EEPROMhasBeenWritten(telegramID, fanAddress, fanGroup);
}

void FFUdatabase::slot_EEPROMdata(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 dataByte)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == nullptr)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        m_loghandler->slot_newEntry(LogEntry::Error, "FFUdatabase::slot_EEPROMdata", "Telegram id mismatch.");
        return;
    }
    ffu->slot_EEPROMdata(telegramID, fanAddress, fanGroup, eepromAddress, dataByte);
}

void FFUdatabase::slot_startFastSpeedPollingSequence()
{
    if (!m_timer_fastSpeedPolling.isActive())
    {
        foreach (EbmBus* ebmBus, *m_ebmbuslist)
            ebmBus->clearTelegramQueue();               // Drop all other request packets from standard priority queue out of the way
    }
    m_timer_fastSpeedPolling.start();
}

void FFUdatabase::slot_timer_pollStatus_fired()
{
//    static int currentFFUid = 0;

//    if (m_ffus.count() > currentFFUid)
//    {
//        FFU* ffu = m_ffus.at(currentFFUid);
//        ffu->requestStatus();
//    }

//    currentFFUid++;
//    if (m_ffus.count() <= currentFFUid)
//        currentFFUid = 0;


    bool actualSpeedOnly = m_timer_fastSpeedPolling.isActive();   // This is set to true if we want to do a fast poll of motor speeds after setpoint change

    foreach (EbmBus* ebmBus, *m_ebmbuslist)
    {
        int sizeOfTelegramQueue = qMax(ebmBus->getSizeOfTelegramQueue(false), ebmBus->getSizeOfTelegramQueue(true));
        if (sizeOfTelegramQueue < 20)
        {
            foreach(FFU* ffu, m_ffus)
            {
                if (m_ebmbuslist->indexOf(ebmBus) == ffu->getBusID())
                {
                    ffu->requestStatus(actualSpeedOnly);
                }
            }
        }
    }
}
