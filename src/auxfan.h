#ifndef AUXFAN_H
#define AUXFAN_H

#include <QObject>
#include <QObject>
#include <QMap>
#include <QDateTime>
#include "ebmmodbussystem.h"
#include "loghandler.h"

class AuxFan : public QObject
{
    Q_OBJECT
public:
    explicit AuxFan(QObject *parent, EbmModbusSystem *ebmModbusSystem, Loghandler* loghandler);
    ~AuxFan();

    typedef struct {
        bool online;
        quint64 lostTelegrams;
        QDateTime lastSeen;
        quint32 speedSettingLostCount;
        quint16 speedReading;
        quint16 speedSetpoint;
        quint16 statusRaw;
        QString statusString;
        quint16 warnings;
        double dcCurrent;
        double dcVoltage;
        double dcPower;
        qint16 temperatureOfPowerModule;
    } ActualData;

    typedef struct {
        bool valid;
        int speedMax;                           // Unit RpM
        int referenceDClinkVoltage;             // Unit 20 mV
        int referenceDClinkCurrent;             // Unit 2 mA
    } ConfigData;

    int getId() const;
    void setId(int id);

    int getBusID() const;
    void setBusID(int busID);

    int getFanAddress() const;
    void setFanAddress(int fanAddress);

    void setSpeed(double rpm);
    void setSpeedRaw(int value, bool refreshOnly = false);

    int getSpeedSetpoint();
    int getSpeedSetpointRaw();

    double rawSpeedToRPM(int rawSpeed);
    int rpmToRawSpeed(double rpm);

    QString getData(QString key);
    void setData(QString key, QString value);

    void setRemoteControlled(bool remoteControlled);
    bool isRemoteControlled() const;

    QStringList getActualKeys();
    ActualData getActualData() const;

    // This function triggers bus requests to get actual values, status, warnings ans errors
    void requestStatus();

    // This function triggers bus requests to get the necessary config data from the ffu
    void requestConfig();

    // This enables automatic startup of the fan. Be carful: If set, each setpoint change results in a writecycle of the eeprom
    void setAutostart(bool enabled);

    void save();
    void setFiledirectory(QString path);
    void load(QString filename);

    void setAutoSave(bool on);

    void deleteFromHdd();
    void deleteAllErrors();

    bool isThisYourTelegram(quint64 telegramID, bool deleteID = true);

private:
    EbmModbusSystem* m_ebmModbusSystem;
    Loghandler* m_loghandler;
    QList<quint64> m_transactionIDs;

    int m_id;
    int m_setpointSpeedRaw;
    double m_speedMaxRPM;
    int m_busID;
    int m_fanAddress;

    bool m_remoteControlled;

    ActualData m_actualData;
    ConfigData m_configData;

    bool m_dataChanged;
    bool m_autosave;
    QString m_filepath;

    QString myFilename();

    bool isConfigured();    // Returns false if either fanAddress or busID is not set
    void markAsOnline();

    void setNmax(double maxRpm);
    void setNmaxFromConfigData();

    // This uses m_configData to configure ffu
    void processConfigData();

signals:
    void signal_needsSaving();
    void signal_FanActualDataHasChanged(int id);

public slots:
    // High level bus response slots
    void slot_transactionLost(quint64 id);
    void slot_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg, quint16 rawdata);
    void slot_receivedInputRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusInputRegister reg, quint16 rawdata);
    void slot_wroteHoldingRegisterData(quint64 telegramID);
};

#endif // AUXFAN_H
