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

#ifndef FFU_H
#define FFU_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include "ebmbussystem.h"
#include "loghandler.h"

class FFU : public QObject
{
    Q_OBJECT
public:
    explicit FFU(QObject *parent, EbmBusSystem *ebmbusSystem, Loghandler* loghandler);
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
        double dcCurrent;
        double dcVoltage;
        quint8 temperatureOfPowerModule;
    } ActualData;

    typedef struct {
        bool valid;
        int speedMax;                           // Unit RpM
        quint8 manufacturingDateCode_Day;
        quint8 manufacturingDateCode_Month;
        quint8 manufacturingDateCode_Year;
        quint32 serialNumber;
        int referenceDClinkVoltage;             // Unit 20 mV
        int referenceDClinkCurrent;             // Unit 2 mA
    } ConfigData;

    int getId() const;
    void setId(int id);

    int getBusID() const;
    void setBusID(int busID);

    int getUnit() const;
    void setUnit(int unit);

    int getFanAddress() const;
    void setFanAddress(int fanAddress);

    int getFanGroup() const;
    void setFanGroup(int fanGroup);

    void setSpeed(int rpm);
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
    EbmBusSystem* m_ebmbusSystem;
    Loghandler* m_loghandler;
    QList<quint64> m_transactionIDs;

    int m_id;
    int m_setpointSpeedRaw;
    double m_speedMaxRPM;
    int m_busID;
    int m_unit; // Position in busline counted from start of bus
    int m_fanAddress;
    int m_fanGroup;

    bool m_remoteControlled;

    ActualData m_actualData;
    ConfigData m_configData;

    bool m_dataChanged;
    bool m_autosave;
    QString m_filepath;

    QString myFilename();

    bool isConfigured();    // Returns false if either fanAddress or fanGroup or busID is not set
    void markAsOnline();

    void setNmax(int maxRpm);
    void setNmaxFromConfigData();

    // This uses m_configData to configure ffu
    void processConfigData();

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
