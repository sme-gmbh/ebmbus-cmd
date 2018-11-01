#ifndef AUXFANDATABASE_H
#define AUXFANDATABASE_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QMap>
#include <QDir>
#include <QDirIterator>
#include <QTimer>
#include <QMap>
#include "ebmmodbussystem.h"
#include "loghandler.h"
#include "auxfan.h"

// AuxFans are managed via Modbus

class AuxFanDatabase : public QObject
{
    Q_OBJECT
public:
    explicit AuxFanDatabase(QObject *parent, EbmModbusSystem *ebmModbusSystem, Loghandler *loghandler);

    void loadFromHdd();
    void saveToHdd();
    QString addAuxFan(int id, int busID, int fanAddress = -1);
    QString deleteAuxFan(int id);

    QList<AuxFan*> getAuxFans(int busNr = -1);    // If busNr is specified only fans of that bus are returned
    AuxFan* getAuxFanByID(int id);

    QString getAuxFanData(int id, QString key);
    QMap<QString,QString> getAuxFanData(int id, QStringList keys);
    QString setAuxFanData(int id, QString key, QString value);
    QString setAuxFanData(int id, QMap<QString,QString> dataMap);

    // Broadcast is not implemented yet
    //QString broadcast(int busID, QMap<QString,QString> dataMap);



private:
    EbmModbusSystem* m_ebmModbusSystem;
    QList<EbmModbus*>* m_ebmModbusList;
    Loghandler* m_loghandler;
    QList<AuxFan*> m_auxfans;
    QTimer m_timer_pollStatus;

    AuxFan* getAuxFanByTelegramID(quint64 telegramID);

signals:
    void signal_AuxFanActualDataHasChanged(int id);

public slots:
    void slot_remoteControlActivated();
    void slot_remoteControlDeactivated();

private slots:
    // High level bus response slots
    void slot_transactionFinished();
    void slot_transactionLost(quint64 telegramID);
    void slot_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg, quint16 rawdata);
    void slot_receivedInputRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusInputRegister reg, quint16 rawdata);
    void slot_wroteHoldingRegisterData(quint64 telegramID);

    //void slot_simpleStatus(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, QString status);
    //void slot_status(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 statusAddress, QString status, quint8 rawValue);
    //void slot_actualSpeed(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 actualRawSpeed);
    //void slot_setPointHasBeenSet(quint64 telegramID, quint8 fanAddress, quint8 fanGroup);
    //void slot_EEPROMhasBeenWritten(quint64 telegramID, quint8 fanAddress, quint8 fanGroup);
    //void slot_EEPROMdata(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 dataByte);

    // Timer slots
    void slot_timer_pollStatus_fired();

};

#endif // AUXFANDATABASE_H
