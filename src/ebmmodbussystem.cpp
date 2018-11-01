#include "ebmmodbussystem.h"

EbmModbusSystem::EbmModbusSystem(QObject *parent, Loghandler *loghandler) : QObject(parent)
{
    m_loghandler = loghandler;

    qRegisterMetaType<EbmModbus::EbmModbusHoldingRegister>("EbmModbus::EbmModbusHoldingRegister");
    qRegisterMetaType<EbmModbus::EbmModbusInputRegister>("EbmModbus::EbmModbusInputRegister");

    // tbd.: Make interface numbers configurable!

    EbmModbus* newEbmModbus = new EbmModbus(0, QString("/dev/ttyUSB3"));    // parent must be 0 in order to be moved to workerThread later
    m_ebmModbuslist.append(newEbmModbus);
    newEbmModbus->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, newEbmModbus, &QObject::deleteLater);

    connect(newEbmModbus, SIGNAL(signal_newEntry(LogEntry::LoggingCategory,QString,QString)), m_loghandler, SLOT(slot_newEntry(LogEntry::LoggingCategory,QString,QString)));
    connect(newEbmModbus, SIGNAL(signal_entryGone(LogEntry::LoggingCategory,QString,QString)), m_loghandler, SLOT(slot_entryGone(LogEntry::LoggingCategory,QString,QString)));
    m_ebmModbuslist.append(newEbmModbus);

    // Routing of calls to bus
    connect(this, &EbmModbusSystem::signal_readHoldingRegisterData, newEbmModbus, &EbmModbus::slot_readHoldingRegisterData);
    connect(this, &EbmModbusSystem::signal_readInputRegisterData, newEbmModbus, &EbmModbus::slot_readInputRegisterData);
    connect(this, &EbmModbusSystem::signal_writeHoldingRegisterData, newEbmModbus, &EbmModbus::slot_writeHoldingRegisterData);

    // Routing of bus results to master
    connect(newEbmModbus, &EbmModbus::signal_transactionLost, this, &EbmModbusSystem::signal_transactionLost);
    connect(newEbmModbus, &EbmModbus::signal_receivedHoldingRegisterData, this, &EbmModbusSystem::signal_receivedHoldingRegisterData);
    connect(newEbmModbus, &EbmModbus::signal_receivedInputRegisterData, this, &EbmModbusSystem::signal_receivedInputRegisterData);
    connect(newEbmModbus, &EbmModbus::signal_wroteHoldingRegisterData, this, &EbmModbusSystem::signal_wroteHoldingRegisterData);

    if (!newEbmModbus->open())
        return;
}

EbmModbusSystem::~EbmModbusSystem()
{
    m_workerThread.quit();
    m_workerThread.wait();
}

QList<EbmModbus *> *EbmModbusSystem::ebmModbuslist()
{
    return (&m_ebmModbuslist);
}

EbmModbus *EbmModbusSystem::getBusByID(int busID)
{
    if (m_ebmModbuslist.length() <= busID)
        return NULL; // Bus id not available

    EbmModbus* bus = m_ebmModbuslist.at(busID);

    return bus;
}

quint64 EbmModbusSystem::readHoldingRegister(int busID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg)
{
    Q_UNUSED(busID);
    quint64 telegramID = getNewTelegramID();
    emit signal_readHoldingRegisterData(telegramID, adr, reg);
    return telegramID;
}

quint64 EbmModbusSystem::writeHoldingRegister(int busID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg, quint16 rawdata)
{
    Q_UNUSED(busID);
    quint64 telegramID = getNewTelegramID();
    emit signal_writeHoldingRegisterData(telegramID, adr, reg, rawdata);
    return telegramID;
}

quint64 EbmModbusSystem::readInputRegister(int busID, quint16 adr, EbmModbus::EbmModbusInputRegister reg)
{
    Q_UNUSED(busID);
    quint64 telegramID = getNewTelegramID();
    emit signal_readInputRegisterData(telegramID, adr, reg);
    return telegramID;
}


quint64 EbmModbusSystem::getNewTelegramID()
{
    static quint64 id = 1;  // Start counting telegram id with 1. 0 is reserved for error
    id++;

    return id;
}

