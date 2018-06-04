#ifndef FFU_H
#define FFU_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include "ebmbussystem.h"

class FFU : public QObject
{
    Q_OBJECT
public:
    explicit FFU(QObject *parent, EbmBusSystem *ebmbusSystem);
    ~FFU();

    typedef struct {
        bool online;
        quint64 lostTelegrams;
        QDateTime lastSeen;
        quint32 speedSettingLostCount;
        quint8 speedReading;
        quint8 speedSetpoint;
        quint8 statusRaw_LSB;
        quint8 statusRaw_MSB;
        QString statusString_LSB;
        QString statusString_MSB;
        quint8 warnings;
        quint8 dcCurrent;
        quint8 dcVoltage;
        quint8 temperatureOfPowerModule;
    } ActualData;

    int getId() const;
    void setId(int id);

    int getBusID() const;
    void setBusID(int busID);

    int getFanAddress() const;
    void setFanAddress(int fanAddress);

    int getFanGroup() const;
    void setFanGroup(int fanGroup);

    void setSpeed(int rpm);
    void setSpeedRaw(int value, bool refreshOnly = false);
    void setMaxRPM(int maxRpm);

    int getSpeedSetpoint();
    int getSpeedSetpointRaw();

    double rawSpeedToRPM(int rawSpeed);
    int rpmToRawSpeed(double rpm);

    QString getData(QString key);
    void setData(QString key, QString value);

    void setRemoteControlled(bool remoteControlled);
    bool isRemoteControlled() const;

    ActualData getActualData() const;

    // This function triggers bus requests to get actual values, status, warnings ans errors
    void requestStatus();

    void save();
    void setFiledirectory(QString path);
    void load(QString filename);

    void setAutoSave(bool on);

    void deleteFromHdd();

    bool isThisYourTelegram(quint64 telegramID, bool deleteID = true);


private:
    EbmBusSystem* m_ebmbusSystem;
    QList<quint64> m_transactionIDs;

    int m_id;
    int m_setpointSpeedRaw;
    double m_speedMaxRPM;
    int m_busID;
    int m_fanAddress;
    int m_fanGroup;

    bool m_remoteControlled;

    ActualData m_actualData;

    bool m_dataChanged;
    bool m_autosave;
    QString m_filepath;

    QString myFilename();

signals:
    void signal_needsSaving();
    void signal_FFUactualDataHasChanged(int id);

public slots:
    // High level bus response slots
    void slot_transactionLost(quint64 id);
    void slot_simpleStatus(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, QString status);
    void slot_status(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 statusAddress, QString status, quint8 rawValue);
    void slot_actualSpeed(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 actualRawSpeed);
    void slot_setPointHasBeenSet(quint64 telegramID, quint8 fanAddress, quint8 fanGroup);
    void slot_EEPROMhasBeenWritten(quint64 telegramID, quint8 fanAddress, quint8 fanGroup);
    void slot_EEPROMdata(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 dataByte);

private slots:
    void slot_save();
};

#endif // FFU_H
