#include "ffudatabase.h"

FFUdatabase::FFUdatabase(QObject *parent, EbmBusSystem *ebmbusSystem) : QObject(parent)
{
    m_ebmbusSystem = ebmbusSystem;
    m_ebmbuslist = ebmbusSystem->ebmbuslist();  // Try to eliminate the use of ebmbuslist here later!

    foreach (EbmBus* ebmBus, *m_ebmbuslist)
    {
        // Bus management connections
        connect(ebmBus, SIGNAL(signal_DaisyChainAdressingFinished()), this, SLOT(slot_DaisyChainAdressingFinished()));

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

    // Timer for cyclic poll task to get the status of ffus
    connect(&m_timer_pollStatus, SIGNAL(timeout()), this, SLOT(slot_timer_pollStatus_fired()));
    m_timer_pollStatus.setInterval(1000);
    m_timer_pollStatus.start();
}

void FFUdatabase::loadFromHdd()
{
    QString directory = "/var/openffucontrol/ffus/";
    QDirIterator iterator(directory, QStringList() << "*.csv", QDir::Files, QDirIterator::NoIteratorFlags);
    while(iterator.hasNext())
    {
        QString filepath = iterator.next();
        FFU* newFFU = new FFU(this, m_ebmbusSystem);
        newFFU->load(filepath);
        newFFU->setFiledirectory(directory);
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

QString FFUdatabase::addFFU(int id, int busID)
{
    FFU* newFFU = new FFU(this, m_ebmbusSystem);
    newFFU->setFiledirectory("/var/openffucontrol/ffus/");
    newFFU->setAutoSave(false);
    newFFU->setId(id);
    newFFU->setBusID(busID);
    newFFU->setAutoSave(true);
    newFFU->save();
    m_ffus.append(newFFU);

    return "OK[FFUdatabase]: Added FFU ID " + QString().setNum(id);
}

QString FFUdatabase::deleteFFU(int id)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == NULL)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    bool ok = m_ffus.removeOne(ffu);
    if (ok)
    {
        ffu->deleteFromHdd();
        delete ffu;
        return "OK[FFUdatabase]: Removed ID " + QString().setNum(id);
    }
    return "Warning[FFUdatabase]: Unable to remove ID " + QString().setNum(id) + " from db.";
}

QList<FFU *> FFUdatabase::getFFUs()
{
    return m_ffus;
}

FFU *FFUdatabase::getFFUbyID(int id)
{
    foreach (FFU* ffu, m_ffus)
    {
        if (ffu->getId() == id)
            return ffu;
    }
    return NULL;    // Not found
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

    return NULL;    // TransactioID not initiated by ffu requests, so it came frome somebody else
}

QString FFUdatabase::getFFUdata(int id, QString key)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == NULL)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    return ffu->getData(key);
}

QMap<QString,QString> FFUdatabase::getFFUdata(int id, QStringList keys)
{
    QMap<QString,QString> response;

    FFU* ffu = getFFUbyID(id);
    if (ffu == NULL)
    {
        return response;
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
    if (ffu == NULL)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    ffu->setData(key, value);
    return "OK[FFUdatabase]: Setting " + key + " to " + value;
}

QString FFUdatabase::setFFUdata(int id, QMap<QString, QString> dataMap)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == NULL)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    QString dataString;

    foreach(QString key, dataMap.keys())
    {
        QString value = dataMap.value(key);
        ffu->setData(key, value);

        dataString.append(" " + key + ":" + value);
    }

    return "OK[FFUdatabase]: Setting data:" + dataString;
}

QString FFUdatabase::startDCIaddressing(int busID, QString startAddress)
{
    if (m_ebmbuslist->count() > busID)
    {
        m_ebmbuslist->at(busID)->startDaisyChainAddressing();
    }
    return "OK[FFUdatabase]: Starting DCI addressing at bus " + QString().setNum(busID) + ". Ignoring startAddress at the moment. Will be fixed later.";
}

QString FFUdatabase::broadcast(int busID, QMap<QString, QString> dataMap)
{
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
        }
        i++;
    }
}

void FFUdatabase::slot_gotResponseRaw(quint64 telegramID, quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data)
{
    Q_UNUSED(telegramID);
    Q_UNUSED(preamble);
    Q_UNUSED(commandAndFanaddress);
    Q_UNUSED(fanGroup);
    Q_UNUSED(data);

    // Dont care about raw responses at the moment
    // These will be parsed by libebmbus anyway and sent to the right highlevel slot
}

void FFUdatabase::slot_transactionFinished()
{

}

void FFUdatabase::slot_transactionLost(quint64 telegramID)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == NULL)
    {
        // Somebody other than the ffu requested that respinse, so do nothing with the response at this point
        return;
    }
    ffu->slot_transactionLost(telegramID);
}

void FFUdatabase::slot_simpleStatus(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, QString status)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == NULL)
    {
        // Somebody other than the ffu requested that respinse, so do nothing with the response at this point
        return;
    }
    ffu->slot_simpleStatus(telegramID, fanAddress, fanGroup, status);
}

void FFUdatabase::slot_status(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 statusAddress, QString status, quint8 rawValue)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == NULL)
    {
        // Somebody other than the ffu requested that respinse, so do nothing with the response at this point
        return;
    }
    ffu->slot_status(telegramID, fanAddress, fanGroup, statusAddress, status, rawValue);
}

void FFUdatabase::slot_actualSpeed(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 actualRawSpeed)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == NULL)
    {
        // Somebody other than the ffu requested that respinse, so do nothing with the response at this point
        return;
    }
    ffu->slot_actualSpeed(telegramID, fanAddress, fanGroup, actualRawSpeed);
}

void FFUdatabase::slot_setPointHasBeenSet(quint64 telegramID, quint8 fanAddress, quint8 fanGroup)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == NULL)
    {
        // Somebody other than the ffu requested that respinse, so do nothing with the response at this point
        return;
    }
    ffu->slot_setPointHasBeenSet(telegramID, fanAddress, fanGroup);
}

void FFUdatabase::slot_EEPROMhasBeenWritten(quint64 telegramID, quint8 fanAddress, quint8 fanGroup)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == NULL)
    {
        // Somebody other than the ffu requested that respinse, so do nothing with the response at this point
        return;
    }
    ffu->slot_EEPROMhasBeenWritten(telegramID, fanAddress, fanGroup);
}

void FFUdatabase::slot_EEPROMdata(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 dataByte)
{
    FFU* ffu = getFFUbyTelegramID(telegramID);
    if (ffu == NULL)
    {
        // Somebody other than the ffu requested that respinse, so do nothing with the response at this point
        return;
    }
    ffu->slot_EEPROMdata(telegramID, fanAddress, fanGroup, eepromAddress, dataByte);
}

void FFUdatabase::slot_timer_pollStatus_fired()
{
    static int currentFFUid = 0;

    if (m_ffus.count() > currentFFUid)
    {
        FFU* ffu = m_ffus.at(currentFFUid);
        ffu->requestStatus();
    }

    currentFFUid++;
    if (m_ffus.count() <= currentFFUid)
        currentFFUid = 0;
}
