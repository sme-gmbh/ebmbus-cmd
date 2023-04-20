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

#ifndef OCUFAN_H
#define OCUFAN_H

#include <QObject>
#include <QObject>
#include <QMap>
#include <QDateTime>
#include "ocumodbussystem.h"
#include "loghandler.h"

class OCUfan : public QObject
{
    Q_OBJECT
public:
    explicit OCUfan(QObject *parent, OcuModbusSystem *ocuModbusSystem, Loghandler* loghandler);
    ~OCUfan();

    #define MODBUS_FFU_BLOCKSIZE 0x10

    //    Modbus_MainbusServer.holdingRegisterWrite(addr_offset + 0x00, temperature_ffu_set[numFFU] * 1000.0);
    //    Modbus_MainbusServer.holdingRegisterWrite(addr_offset + 0x01, pump_set[numFFU] * 1000.0);
    //    Modbus_MainbusServer.holdingRegisterWrite(addr_offset + 0x02, fan_rpm_set[numFFU]);
    //    Modbus_MainbusServer.holdingRegisterWrite(addr_offset + 0x03, Kp_set[numFFU] * 100.0);
    //    Modbus_MainbusServer.holdingRegisterWrite(addr_offset + 0x04, Tn_set[numFFU] * 10.0);
    typedef enum {
        HOLDING_REG_0000_TemperatureFFUSetpoint = 0x0000,
        HOLDING_REG_0001_PumpSetpoint = 0x0001,
        HOLDING_REG_0002_FanRpmSetpoint = 0x0002,
        HOLDING_REG_0003_TemperatureControlKp = 0x0003,
        HOLDING_REG_0004_TemperatureControlTn = 0x0004
    } OcuFanModbusHoldingRegister;

//    Modbus_MainbusServer.inputRegisterWrite(addr_offset + 0x00, (uint16_t) (temperature_ffu_read[numFFU] * 1000.0));
//    Modbus_MainbusServer.inputRegisterWrite(addr_offset + 0x01, (uint16_t) (valve_set[numFFU] * 10000.0));
//    Modbus_MainbusServer.inputRegisterWrite(addr_offset + 0x02, (uint16_t) (valve_read[numFFU] * 10000.0));
//    Modbus_MainbusServer.inputRegisterWrite(addr_offset + 0x03, (uint16_t) (fan_errorCode[numFFU]));
//    Modbus_MainbusServer.inputRegisterWrite(addr_offset + 0x04, (uint16_t) (fan_rpm_read[numFFU]));
//    Modbus_MainbusServer.inputRegisterWrite(addr_offset + 0x05, (uint16_t) (fan_power_read[numFFU]));

    typedef enum {
        INPUT_REG_0000_TemperatureFFURead = 0x0000,
        INPUT_REG_0001_CoolingValveSetpoint = 0x0001,
        INPUT_REG_0002_CoolingValveRead = 0x0002,
        INPUT_REG_0003_FanErrorCode = 0x0003,
        INPUT_REG_0004_FanRpmRead = 0x0004,
        INPUT_REG_0005_FanPowerRead = 0x0005
    } OcuFanModbusInputRegister;

    typedef struct {
        bool online;
        quint64 lostTelegrams;
        QDateTime lastSeen;
        quint32 speedSettingLostCount;
        double speedReading;
        double speedSetpoint;
        QString statusString;
        double fanPower;
        double temperatureRead;
        double temperatureSetpoint;
        double temperatureKp;
        double temperatureTn;
        double coolingValveSet;
        double coolingValveRead;
        double pumpSetpoint;
    } ActualData;

    typedef struct {
        bool valid;                             // True if readback from device was successful
        int speedMax;                           // Unit RpM
    } ConfigData;

    // Central id from the openFFUcontrol database
    int getId() const;
    void setId(int id);

    // Number of the RS485 bus the OCU is connected to
    int getBusID() const;
    void setBusID(int busID);

    // Number of FFU connected to that particular OCU
    // This is used as offset Address on the Modbus starting from 0
    int getFanAddress() const;
    void setFanAddress(int fanAddress);

    // ModBus Address of the current OCU (that may be controlling more than one FFU)
    int getFanGroup() const;
    void setFanGroup(int fanGroup);

    // Set the speed of the fan
    void setSpeed(double rpm);
    void setSpeedRaw(int value, bool refreshOnly = false);

    double rawSpeedToRPM(int rawSpeed);
    int rpmToRawSpeed(double rpm);

    // Set the temperature setpoint
    void setTemperature(double temperature);

    // Set the temperature controller Kp
    void setTemperatureKp(double kp);

    // Set the temperature controller Tn
    void setTemperatureTn(double Tn);

    void setPump(double speed);

    // Get the speed setpoint of the fan
    double getSpeedSetpoint();
    int getSpeedSetpointRaw();

    // Get or set any data by name
    QString getData(QString key);
    void setData(QString key, QString value);

    // Get or set if FFU is remote controlled, so if, the current settings of the device are
    // checked on a regular basis and set accordingly if incorrect
    void setRemoteControlled(bool remoteControlled);
    bool isRemoteControlled() const;

    // Get a list of data keys that this FFU can provide
    QStringList getActualKeys();

    // Get all actual data that this FFU can provide
    ActualData getActualData() const;

    // This function triggers bus requests to get actual values, status, warnings ans errors
    void requestStatus();

    // This function triggers bus requests to get the necessary config data from the ffus
    void requestConfig();

    // This enables automatic startup of the fan. Be carful: If set, each setpoint change results in a writecycle of the eeprom
    // Not autostarting is not supported in OCU controlled systems
//    void setAutostart(bool enabled);

    // Save the setpoints and config to file
    void save();
    void setFiledirectory(QString path);

    // Load setpoints and config from file
    void load(QString filename);

    // Set if changes of important setpoints and config should automatically updated on file if changed
    void setAutoSave(bool on);

    void deleteFromHdd();

    // Tell loghandler that Errors are gone
    void deleteAllErrors();

    // Check if a modbus telegram id corresponds to a request from this FFU
    bool isThisYourTelegram(quint64 telegramID, bool deleteID = true);

private:
    OcuModbusSystem* m_ocuModbusSystem;
    Loghandler* m_loghandler;
    QList<quint64> m_transactionIDs;

    int m_id;
    int m_setpointSpeedRaw;
    double m_speedMaxRPM;
    int m_busID;
    int m_fanAddress;   // Modbus Data Address offset counter
    int m_fanGroup;     // Modbus Unit Address

    int m_setpointTemperatureRaw;
    int m_temperatureKpRaw;
    int m_temperatureTnRaw;
    int m_setpointPumpRaw;

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
    void slot_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, quint16 reg, QList<quint16> data);
    void slot_receivedInputRegisterData(quint64 telegramID, quint16 adr, quint16 reg, QList<quint16> data);

private slots:
    void slot_save();
};

#endif // OCUFAN_H

