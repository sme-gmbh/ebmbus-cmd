#include "auxfandatabase.h"

AuxFanDatabase::AuxFanDatabase(QObject *parent,  EbmModbusSystem *ebmModbusSystem, Loghandler *loghandler) : QObject(parent)
{
    m_ebmModbusSystem = ebmModbusSystem;

    m_ebmModbusList = ebmModbusSystem->ebmModbuslist(); // Try to eliminate this!

    m_loghandler = loghandler;

    // High level bus-system response connections
    connect(m_ebmModbusSystem, &EbmModbusSystem::signal_receivedHoldingRegisterData, this, &AuxFanDatabase::slot_receivedHoldingRegisterData);
    connect(m_ebmModbusSystem, &EbmModbusSystem::signal_receivedInputRegisterData, this, &AuxFanDatabase::slot_receivedInputRegisterData);
    connect(m_ebmModbusSystem, &EbmModbusSystem::signal_wroteHoldingRegisterData, this, &AuxFanDatabase::slot_wroteHoldingRegisterData);
    connect(m_ebmModbusSystem, &EbmModbusSystem::signal_transactionLost, this, &AuxFanDatabase::slot_transactionLost);

    connect(&m_timer_pollStatus, &QTimer::timeout, this, &AuxFanDatabase::slot_timer_pollStatus_fired);
    m_timer_pollStatus.setInterval(2000);
    m_timer_pollStatus.start();
}

void AuxFanDatabase::loadFromHdd()
{
    QString directory = "/var/openffucontrol/auxfans/";
    QDirIterator iterator(directory, QStringList() << "*.csv", QDir::Files, QDirIterator::NoIteratorFlags);
    while(iterator.hasNext())
    {
        QString filepath = iterator.next();
        AuxFan* newAuxFan = new AuxFan(this, m_ebmModbusSystem, m_loghandler);
        newAuxFan->load(filepath);
        newAuxFan->setFiledirectory(directory);
        connect(newAuxFan, &AuxFan::signal_FanActualDataHasChanged, this, &AuxFanDatabase::signal_AuxFanActualDataHasChanged);
        m_auxfans.append(newAuxFan);
    }
}

void AuxFanDatabase::saveToHdd()
{
    QString path = "/var/openffucontrol/auxfans/";

    foreach (AuxFan* auxFan, m_auxfans)
    {
        auxFan->setFiledirectory(path);
        auxFan->save();
    }
}

QString AuxFanDatabase::addAuxFan(int id, int busID, int fanAddress)
{
    AuxFan* newAuxFan = new AuxFan(this, m_ebmModbusSystem, m_loghandler);
    newAuxFan->setFiledirectory("/var/openffucontrol/auxfans/");
    newAuxFan->setAutoSave(false);
    newAuxFan->setId(id);
    newAuxFan->setBusID(busID);
    newAuxFan->setFanAddress(fanAddress);
    newAuxFan->setAutoSave(true);
    newAuxFan->save();
    connect(newAuxFan, &AuxFan::signal_FanActualDataHasChanged, this, &AuxFanDatabase::signal_AuxFanActualDataHasChanged);
    m_auxfans.append(newAuxFan);

    return "OK[AuxFanDatabase]: Added AuxFan ID " + QString().setNum(id);
}

QString AuxFanDatabase::deleteAuxFan(int id)
{
    AuxFan* auxFan = getAuxFanByID(id);
    if (auxFan == NULL)
        return "Warning[AuxFanDatabase]: ID " + QString().setNum(id) + " not found.";

    bool ok = m_auxfans.removeOne(auxFan);
    if (ok)
    {
        disconnect(auxFan, &AuxFan::signal_FanActualDataHasChanged, this, &AuxFanDatabase::signal_AuxFanActualDataHasChanged);
        auxFan->deleteFromHdd();
        auxFan->deleteAllErrors();
        delete auxFan;
        return "OK[AuxFanDatabase]: Removed ID " + QString().setNum(id);
    }
    return "Warning[AuxFanDatabase]: Unable to remove ID " + QString().setNum(id) + " from db.";
}

QList<AuxFan *> AuxFanDatabase::getAuxFans(int busNr)
{
    QList<AuxFan *> auxFanList;
    if (busNr == -1)
        return m_auxfans;
    else
    {
        foreach (AuxFan* auxFan, m_auxfans) {
            if (auxFan->getBusID() == busNr)
                auxFanList.append(auxFan);
        }
    }

    return auxFanList;
}

AuxFan *AuxFanDatabase::getAuxFanByID(int id)
{
    foreach (AuxFan* auxFan, m_auxfans)
    {
        if (auxFan->getId() == id)
            return auxFan;
    }
    return NULL;    // Not found
}

AuxFan *AuxFanDatabase::getAuxFanByTelegramID(quint64 telegramID)
{
    foreach (AuxFan* auxFan, m_auxfans) {
        bool found = auxFan->isThisYourTelegram(telegramID);
        if (found)
        {
            return auxFan;
        }
    }

    return NULL;    // TransactionID not initiated by ffu requests, so it came frome somebody else
}

QString AuxFanDatabase::getAuxFanData(int id, QString key)
{
    AuxFan* auxFan = getAuxFanByID(id);
    if (auxFan == NULL)
        return "Warning[AuxFanDatabase]: ID " + QString().setNum(id) + " not found.";

    return auxFan->getData(key);
}

QMap<QString, QString> AuxFanDatabase::getAuxFanData(int id, QStringList keys)
{
    QMap<QString,QString> response;

    AuxFan* auxFan = getAuxFanByID(id);
    if (auxFan == NULL)
    {
        return response;
    }

    if (keys.contains("actual"))
    {
        keys.clear();   // Only show actual values, drop all other requests because the answer goes into a special processing later
        keys.append(auxFan->getActualKeys());
        response.insert("actualData", "1"); // And show the recipient that this is actualData
    }

    foreach (QString key, keys)
    {
        response.insert(key, auxFan->getData(key));
    }

    return response;
}

QString AuxFanDatabase::setAuxFanData(int id, QString key, QString value)
{
    AuxFan* auxFan = getAuxFanByID(id);
    if (auxFan == NULL)
        return "Warning[AuxFanDatabase]: ID " + QString().setNum(id) + " not found.";

    auxFan->setData(key, value);
    return "OK[AuxFanDatabase]: Setting " + key + " to " + value;
}

QString AuxFanDatabase::setAuxFanData(int id, QMap<QString, QString> dataMap)
{
    AuxFan* auxFan = getAuxFanByID(id);
    if (auxFan == NULL)
        return "Warning[AuxFanDatabase]: ID " + QString().setNum(id) + " not found.";

    QString dataString;

    foreach(QString key, dataMap.keys())
    {
        QString value = dataMap.value(key);
        auxFan->setData(key, value);

        dataString.append(" " + key + ":" + value);
    }

    return "OK[AuxFanDatabase]: Setting data:" + dataString;
}

void AuxFanDatabase::slot_remoteControlActivated()
{
    foreach (AuxFan* auxFan, m_auxfans) {
        auxFan->setRemoteControlled(true);
    }
}

void AuxFanDatabase::slot_remoteControlDeactivated()
{
    foreach (AuxFan* auxFan, m_auxfans) {
        auxFan->setRemoteControlled(false);
    }
}

void AuxFanDatabase::slot_transactionFinished()
{
    // Do nothing
}

void AuxFanDatabase::slot_transactionLost(quint64 telegramID)
{
    AuxFan* auxFan = getAuxFanByTelegramID(telegramID);
    if (auxFan == NULL)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    auxFan->slot_transactionLost(telegramID);
}

void AuxFanDatabase::slot_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg, quint16 rawdata)
{
    AuxFan* auxFan = getAuxFanByTelegramID(telegramID);
    if (auxFan == NULL)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    auxFan->slot_receivedHoldingRegisterData(telegramID, adr, reg, rawdata);
}

void AuxFanDatabase::slot_receivedInputRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusInputRegister reg, quint16 rawdata)
{
    AuxFan* auxFan = getAuxFanByTelegramID(telegramID);
    if (auxFan == NULL)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    auxFan->slot_receivedInputRegisterData(telegramID, adr, reg, rawdata);
}

void AuxFanDatabase::slot_wroteHoldingRegisterData(quint64 telegramID)
{
    AuxFan* auxFan = getAuxFanByTelegramID(telegramID);
    if (auxFan == NULL)
    {
        // Somebody other than the ffu requested that response, so do nothing with the response at this point
        return;
    }
    auxFan->slot_wroteHoldingRegisterData(telegramID);
}

void AuxFanDatabase::slot_timer_pollStatus_fired()
{
    static int currentAuxFanId = 0;

    if (m_auxfans.count() > currentAuxFanId)
    {
        AuxFan* auxFan = m_auxfans.at(currentAuxFanId);
        auxFan->requestStatus();
    }

    currentAuxFanId++;
    if (m_auxfans.count() <= currentAuxFanId)
        currentAuxFanId = 0;
}