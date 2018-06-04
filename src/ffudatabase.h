#ifndef FFUDATABASE_H
#define FFUDATABASE_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QMap>
#include <QDir>
#include <QDirIterator>
#include <QTimer>
#include <QMap>
#include <libebmbus/ebmbus.h>
#include "ffu.h"
#include "ebmbussystem.h"

class FFUdatabase : public QObject
{
    Q_OBJECT
public:
    explicit FFUdatabase(QObject *parent, EbmBusSystem* ebmbusSystem);

    void loadFromHdd();
    void saveToHdd();
    QString addFFU(int id, int busID, int unit = -1, int fanAddress = -1, int fanGroup = -1);
    QString deleteFFU(int id);

    QList<FFU*> getFFUs(int busNr = -1);    // If busNr is specified only ffus of that bus are returned
    FFU* getFFUbyID(int id);

    QString getFFUdata(int id, QString key);
    QMap<QString,QString> getFFUdata(int id, QStringList keys);
    QString setFFUdata(int id, QString key, QString value);
    QString setFFUdata(int id, QMap<QString,QString> dataMap);

    QString startDCIaddressing(int busID, QString startAddress, QString idsString);

    QString broadcast(int busID, QMap<QString,QString> dataMap);

private:
    EbmBusSystem* m_ebmbusSystem;
    QList<EbmBus*>* m_ebmbuslist;
    QList<FFU*> m_ffus;
    QTimer m_timer_pollStatus;
    QMap<int,QList<int>> m_unitIdsPerBus;

    FFU* getFFUbyTelegramID(quint64 telegramID);

signals:
    void signal_DCIaddressingFinished(int busID);
    void signal_DCIaddressingGotSerialNumber(int busID, quint8 unit, quint8 fanAddress, quint8 fanGroup, quint32 serialNumber);
    void signal_FFUactualDataHasChanged(int id);

public slots:
    void slot_remoteControlActivated();
    void slot_remoteControlDeactivated();

private slots:
    // Bus management response slots
    void slot_DaisyChainAdressingFinished();

    // Low level bus response slots
    void slot_gotResponseRaw(quint64 telegramID, quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data);
    void slot_DaisyChainAddressingGotSerialNumber(quint8 unit, quint8 fanAddress, quint8 fanGroup, quint32 serialNumber);

    // High level bus response slots
    void slot_transactionFinished();
    void slot_transactionLost(quint64 telegramID);
    void slot_simpleStatus(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, QString status);
    void slot_status(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 statusAddress, QString status, quint8 rawValue);
    void slot_actualSpeed(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 actualRawSpeed);
    void slot_setPointHasBeenSet(quint64 telegramID, quint8 fanAddress, quint8 fanGroup);
    void slot_EEPROMhasBeenWritten(quint64 telegramID, quint8 fanAddress, quint8 fanGroup);
    void slot_EEPROMdata(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 dataByte);

    // Timer slots
    void slot_timer_pollStatus_fired();
};

#endif // FFUDATABASE_H
