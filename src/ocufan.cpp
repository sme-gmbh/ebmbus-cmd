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

#include "ocufan.h"

#include <QFile>
#include <QString>
#include <QStringList>
#include <QDir>

OCUfan::OCUfan(QObject *parent, OcuModbusSystem *ocuModbusSystem, Loghandler* loghandler) : QObject(parent)
{
    m_ocuModbusSystem = ocuModbusSystem;
    m_loghandler = loghandler;

    m_dataChanged = false;
    setAutoSave(true);

    m_id = -1;
    m_setpointSpeedRaw = 720;
    m_speedMaxRPM = 1270;     // Just some initial data, read back real values later from EEPROM
    m_setpointTemperatureRaw = 20000;   // 20.000°C
    m_temperatureKpRaw = -1 * 100;            // -1
    m_temperatureTnRaw = 360 * 10;           // 360
    m_setpointPumpRaw = 1000;   // 100 %
    m_busID = -1;       // Invalid bus
    m_fanAddress = -1;  // Invalid Address
    m_fanGroup = -1;     // Invalid Address

    m_remoteControlled = true;

    m_actualData.online = false;
    m_actualData.lostTelegrams = 0;
    m_actualData.lastSeen = QDateTime();
    m_actualData.speedSettingLostCount = 0;
    m_actualData.speedReading = 0.0;
    m_actualData.speedSetpoint = 0.0;
    m_actualData.fanPower = 0.0;
    m_actualData.temperatureRead = 0.0;
    m_actualData.temperatureSetpoint = 0.0;
    m_actualData.temperatureKp = 0.0;
    m_actualData.temperatureTn = 0.0;
    m_actualData.coolingValveSet = 0.0;
    m_actualData.coolingValveRead = 0.0;

    m_configData.valid = false;
    m_configData.speedMax = 0;
}

OCUfan::~OCUfan()
{

}

int OCUfan::getId() const
{
    return m_id;
}

void OCUfan::setId(int id)
{
    if (id != m_id)
    {
        m_id = id;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

int OCUfan::getBusID() const
{
    return m_busID;
}

void OCUfan::setBusID(int busID)
{
    if (busID != m_busID)
    {
        m_busID = busID;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

int OCUfan::getFanAddress() const
{
    return m_fanAddress;
}

void OCUfan::setFanAddress(int fanAddress)
{
    if (m_fanAddress != fanAddress)
    {
        m_fanAddress = fanAddress;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

int OCUfan::getFanGroup() const
{
    return m_fanGroup;
}

void OCUfan::setFanGroup(int fanGroup)
{
    if (m_fanGroup != fanGroup)
    {
        m_fanGroup = fanGroup;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

void OCUfan::setSpeed(double rpm)
{
    int setpointSpeedRaw = this->rpmToRawSpeed(rpm);
    if (m_setpointSpeedRaw != setpointSpeedRaw)
    {
        setSpeedRaw(setpointSpeedRaw);
    }
}

void OCUfan::setSpeedRaw(int value, bool refreshOnly)
{
    if ((value != m_setpointSpeedRaw) || refreshOnly)
    {
        if (!refreshOnly)
        {
            m_setpointSpeedRaw = value;
            m_dataChanged = true;
            emit signal_needsSaving();
        }
        if (isConfigured())
        {
            OcuModbus* bus = m_ocuModbusSystem->getBusByID(m_busID);
            if (bus == nullptr)
                return;     // Drop requests for non existing bus ids

            m_transactionIDs.append(m_ocuModbusSystem->writeHoldingRegister(m_busID, m_fanGroup, OCUfan::HOLDING_REG_0002_FanRpmSetpoint + MODBUS_FFU_BLOCKSIZE * m_fanAddress, m_setpointSpeedRaw));
        }
        else
        {
            // Some error handling here!
        }
    }
}

double OCUfan::rawSpeedToRPM(int rawSpeed)
{
    return ((double)rawSpeed);
}

int OCUfan::rpmToRawSpeed(double rpm)
{
    return (int)(rpm);
}

void OCUfan::setTemperature(double temperature)
{
    m_setpointTemperatureRaw = (int)(temperature * 1000.0);

    if (isConfigured())
    {
        OcuModbus* bus = m_ocuModbusSystem->getBusByID(m_busID);
        if (bus == nullptr)
            return;     // Drop requests for non existing bus ids

        m_transactionIDs.append(m_ocuModbusSystem->writeHoldingRegister(m_busID, m_fanGroup, OCUfan::HOLDING_REG_0000_TemperatureFFUSetpoint + MODBUS_FFU_BLOCKSIZE * m_fanAddress, m_setpointTemperatureRaw));
    }
    else
    {
        // Some error handling here!
    }
}

void OCUfan::setTemperatureKp(double Kp)
{
    m_temperatureKpRaw = (int)(Kp * 100.0);
    if (isConfigured())
    {
        OcuModbus* bus = m_ocuModbusSystem->getBusByID(m_busID);
        if (bus == nullptr)
            return;     // Drop requests for non existing bus ids

        m_transactionIDs.append(m_ocuModbusSystem->writeHoldingRegister(m_busID, m_fanGroup, OCUfan::HOLDING_REG_0003_TemperatureControlKp + MODBUS_FFU_BLOCKSIZE * m_fanAddress, m_temperatureKpRaw));
    }
    else
    {
        // Some error handling here!
    }
}

void OCUfan::setTemperatureTn(double Tn)
{
    m_temperatureTnRaw = (int)(Tn * 10.0);
    if (isConfigured())
    {
        OcuModbus* bus = m_ocuModbusSystem->getBusByID(m_busID);
        if (bus == nullptr)
            return;     // Drop requests for non existing bus ids

        m_transactionIDs.append(m_ocuModbusSystem->writeHoldingRegister(m_busID, m_fanGroup, OCUfan::HOLDING_REG_0004_TemperatureControlTn + MODBUS_FFU_BLOCKSIZE * m_fanAddress, m_temperatureTnRaw));
    }
    else
    {
        // Some error handling here!
    }
}

void OCUfan::setPump(double speed)
{
    m_setpointPumpRaw = (int)(speed * 1000.0);
    if (isConfigured())
    {
        OcuModbus* bus = m_ocuModbusSystem->getBusByID(m_busID);
        if (bus == nullptr)
            return;     // Drop requests for non existing bus ids

        m_transactionIDs.append(m_ocuModbusSystem->writeHoldingRegister(m_busID, m_fanGroup, OCUfan::HOLDING_REG_0004_TemperatureControlTn + MODBUS_FFU_BLOCKSIZE * m_fanAddress, m_temperatureTnRaw));
    }
    else
    {
        // Some error handling here!
    }
}

double OCUfan::getSpeedSetpoint()
{
    return (rawSpeedToRPM(m_setpointSpeedRaw));
}

int OCUfan::getSpeedSetpointRaw()
{
    return m_setpointSpeedRaw;
}

QString OCUfan::getData(QString key)
{
    // ***** Static keys *****
    if (key == "id")
    {
        return (QString().setNum(m_id));
    }
    else if (key == "nSet")
    {
        return (QString().setNum(getSpeedSetpoint()));
    }
    else if (key == "rawspeed")
    {
        return (QString().setNum(getSpeedSetpointRaw()));
    }
    else if (key == "busID")
    {
        return (QString().setNum(getBusID()));
    }
    else if (key == "fanAddress")
    {
        return (QString().setNum(getFanAddress()));
    }
    else if (key == "fanGroup")
    {
        return (QString().setNum(getFanGroup()));
    }
    // ***** Actual keys *****
    else if (key == "online")
    {
        return QString().sprintf("%i", m_actualData.online);
    }
    else if (key == "lostTelegrams")
    {
        return QString().sprintf("%lli", m_actualData.lostTelegrams);
    }
    else if (key == "lastSeen")
    {
        return m_actualData.lastSeen.toString("yyyy.MM.dd-hh:mm:ss.zzz");
    }
    else if (key == "speedSettingLostCount")
    {
        return QString().sprintf("%i", m_actualData.speedSettingLostCount);
    }
    else if (key == "speedReading")
    {
        return QString().sprintf("%i", m_actualData.speedReading);
    }
    else if (key == "speedSetpoint")
    {
        return QString().sprintf("%i", m_actualData.speedSetpoint);
    }
    else if (key == "fanPower")
    {
        return QString().sprintf("%.0lf", m_actualData.fanPower);
    }
    else if (key == "temperatureRead")
    {
        return QString().sprintf("%.3lf", m_actualData.temperatureRead);
    }
    else if (key == "temperatureSetpoint")
    {
        return QString().sprintf("%.3lf", m_actualData.temperatureSetpoint);
    }
    else if (key == "temperatureKp")
    {
        return QString().sprintf("%lf", m_actualData.temperatureKp);
    }
    else if (key == "temperatureTn")
    {
        return QString().sprintf("%lf", m_actualData.temperatureTn);
    }
    else if (key == "coolingValveSet")
    {
        return QString().sprintf("%.3lf", m_actualData.coolingValveSet);
    }
    else if (key == "coolingValveRead")
    {
        return QString().sprintf("%.3lf", m_actualData.coolingValveRead);
    }
    else if (key == "pumpSetpoint")
    {
        return QString().sprintf("%.3lf", m_actualData.pumpSetpoint);
    }

    return "Error[AuxFan]: Key " + key + " not available";
}

void OCUfan::setData(QString key, QString value)
{
    if (key == "nSet")
    {
        setSpeed(value.toDouble());
    }
    else if (key == "rawspeed")
    {
        setSpeedRaw(value.toInt());
    }
    else if (key == "busID")
    {
        setBusID(value.toInt());
    }
    else if (key == "fanAddress")
    {
        setFanAddress(value.toInt());
    }
    else if (key == "fanGroup")
    {
        setFanGroup(value.toInt());
    }
    else if (key =="temperatureSetpoint")
    {
        setTemperature(value.toDouble());
    }
    else if (key =="temperatureKp")
    {
        setTemperatureKp(value.toDouble());
    }
    else if (key =="temperatureTn")
    {
        setTemperatureTn(value.toDouble());
    }
    else if (key =="pumpSetpoint")
    {
        setPump(value.toDouble());
    }
}

void OCUfan::setRemoteControlled(bool remoteControlled)
{
    m_remoteControlled = remoteControlled;
//    if (remoteControlled)
//        setAutostart(true);
}

bool OCUfan::isRemoteControlled() const
{
    return m_remoteControlled;
}

QStringList OCUfan::getActualKeys()
{
    QStringList keys;

    keys += "online";
    keys += "lostTelegrams";
    keys += "lastSeen";
    keys += "speedSettingLostCount";
    keys += "speedReading";
    keys += "speedSetpoint";
    keys += "fanPower";
    keys += "temperatureRead";
    keys += "temperatureSetpoint";
    keys += "temperatureKp";
    keys += "temperatureTn";
    keys += "coolingValveSet";
    keys += "coolingValveRead";
    keys += "pumpSetpoint";

    return keys;
}

OCUfan::ActualData OCUfan::getActualData() const
{
    return m_actualData;
}

void OCUfan::requestStatus()
{
    if (!isConfigured())
    {
        return;
    }

    OcuModbus* bus = m_ocuModbusSystem->getBusByID(m_busID);
    if (bus == nullptr)
    {
        return;
    }

    if (!m_configData.valid)
        requestConfig();

    m_transactionIDs.append(m_ocuModbusSystem->readHoldingRegister(m_busID, m_fanGroup, OCUfan::HOLDING_REG_0004_TemperatureControlTn + m_fanAddress * MODBUS_FFU_BLOCKSIZE));
    m_transactionIDs.append(m_ocuModbusSystem->readHoldingRegister(m_busID, m_fanGroup, OCUfan::HOLDING_REG_0003_TemperatureControlKp + m_fanAddress * MODBUS_FFU_BLOCKSIZE));
    m_transactionIDs.append(m_ocuModbusSystem->readHoldingRegister(m_busID, m_fanGroup, OCUfan::HOLDING_REG_0002_FanRpmSetpoint + m_fanAddress * MODBUS_FFU_BLOCKSIZE));
    m_transactionIDs.append(m_ocuModbusSystem->readHoldingRegister(m_busID, m_fanGroup, OCUfan::HOLDING_REG_0001_PumpSetpoint + m_fanAddress * MODBUS_FFU_BLOCKSIZE));
    m_transactionIDs.append(m_ocuModbusSystem->readHoldingRegister(m_busID, m_fanGroup, OCUfan::HOLDING_REG_0000_TemperatureFFUSetpoint + m_fanAddress * MODBUS_FFU_BLOCKSIZE));
    m_transactionIDs.append(m_ocuModbusSystem->readInputRegister(m_busID, m_fanGroup, OCUfan::INPUT_REG_0005_FanPowerRead + m_fanAddress * MODBUS_FFU_BLOCKSIZE));
    m_transactionIDs.append(m_ocuModbusSystem->readInputRegister(m_busID, m_fanGroup, OCUfan::INPUT_REG_0004_FanRpmRead + m_fanAddress * MODBUS_FFU_BLOCKSIZE));
    m_transactionIDs.append(m_ocuModbusSystem->readInputRegister(m_busID, m_fanGroup, OCUfan::INPUT_REG_0003_FanErrorCode + m_fanAddress * MODBUS_FFU_BLOCKSIZE));
    m_transactionIDs.append(m_ocuModbusSystem->readInputRegister(m_busID, m_fanGroup, OCUfan::INPUT_REG_0002_CoolingValveRead + m_fanAddress * MODBUS_FFU_BLOCKSIZE));
    m_transactionIDs.append(m_ocuModbusSystem->readInputRegister(m_busID, m_fanGroup, OCUfan::INPUT_REG_0001_CoolingValveSetpoint + m_fanAddress * MODBUS_FFU_BLOCKSIZE));
    m_transactionIDs.append(m_ocuModbusSystem->readInputRegister(m_busID, m_fanGroup, OCUfan::INPUT_REG_0000_TemperatureFFURead + m_fanAddress * MODBUS_FFU_BLOCKSIZE));
}

void OCUfan::requestConfig()
{
    if (!isConfigured())
        return;

    OcuModbus* bus = m_ocuModbusSystem->getBusByID(m_busID);
    if (bus == nullptr)
        return;

//    m_transactionIDs.append(m_ocuModbusSystem->readHoldingRegister(m_busID, m_fanGroup, OCUfan::HOLDING_REG_0000_TemperatureFFUSetpoint));
    // Last Request DC link voltage triggers processing of config data if response telegram is processed
//    m_transactionIDs.append(m_ebmModbusSystem->readHoldingRegister(m_busID, m_fanAddress, EbmModbus::HOLDING_REG_D1A0_ReferenceValueOfDClinkVoltage));

}

void OCUfan::save()
{
    if (!m_dataChanged)
        return;

    QFile file(myFilename());
    if (!file.open(QIODevice::WriteOnly))
        return;

    QString wdata;

    wdata.append(QString().sprintf("id=%i;", m_id));
    wdata.append(QString().sprintf("bus=%i;", m_busID));
    wdata.append(QString().sprintf("fanAddress=%i;", m_fanAddress));
    wdata.append(QString().sprintf("fanGroup=%i;", m_fanGroup));
    wdata.append(QString().sprintf("setpointSpeedRaw=%i;", m_setpointSpeedRaw));
    wdata.append(QString().sprintf("speedSettingLostCount=%i;", m_actualData.speedSettingLostCount));  // Todo: decide when to save this. Do not write flash to dead!
    wdata.append(QString().sprintf("temperatureSetPointRaw=%i;", m_setpointTemperatureRaw));
    wdata.append(QString().sprintf("pumpSetPointRaw=%i;", m_setpointPumpRaw));
    wdata.append(QString().sprintf("temperatureKpRaw=%i;", m_temperatureKpRaw));
    wdata.append(QString().sprintf("temperatureTnRaw=%i\n", m_temperatureTnRaw));

    file.write(wdata.toUtf8());

    file.close();
}

void OCUfan::setFiledirectory(QString path)
{
    if (!path.endsWith("/"))
        path.append("/");
    m_filepath = path;

    QDir dir;
    dir.mkpath(path);
}

void OCUfan::load(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
        return;

    QString rdata = QString().fromUtf8(file.readLine());

    QStringList dataList = rdata.split(";");
    foreach (QString data, dataList)
    {
        QStringList pair = data.split("=");
        if (pair.count() != 2)
            continue;

        QString key = pair.at(0);
        QString value = pair.at(1);

        if (key == "id")
            m_id = value.toInt();

        if (key == "bus")
            m_busID = value.toInt();

        if (key == "fanAddress")
            m_fanAddress = value.toInt();

        if (key == "fanGroup")
            m_fanGroup = value.toInt();

        if (key == "setpointSpeedRaw")
            m_setpointSpeedRaw = value.toInt();

        if (key == "speedSettingLostCount")
        {
            m_actualData.speedSettingLostCount = value.toInt();
        }

        if (key == "temperatureSetPointRaw")
        {
            m_setpointTemperatureRaw = value.toInt();
        }

        if (key == "pumpSetPointRaw")
        {
            m_setpointPumpRaw = value.toInt();
        }

        if (key == "temperatureKpRaw")
        {
            m_temperatureKpRaw = value.toInt();
        }

        if (key == "temperatureTnRaw")
        {
            m_temperatureTnRaw = value.toInt();
        }
    }

    file.close();
}

void OCUfan::setAutoSave(bool on)
{
    m_autosave = on;
    if (m_autosave)
        connect(this, SIGNAL(signal_needsSaving()), this, SLOT(slot_save()));
    else
        disconnect(this, SIGNAL(signal_needsSaving()), this, SLOT(slot_save()));
}

void OCUfan::deleteFromHdd()
{
    QFile file(myFilename());
    file.remove();
}

void OCUfan::deleteAllErrors()
{
    m_loghandler->slot_entryGone(LogEntry::Error, "OCUfan id=" + QString().setNum(m_id), "Not online.");
    m_loghandler->slot_entryGone(LogEntry::Warning, "OCUfan id=" + QString().setNum(m_id), "Warnings present.");
}

bool OCUfan::isThisYourTelegram(quint64 telegramID, bool deleteID)
{
    bool found = m_transactionIDs.contains(telegramID);

    if (found && deleteID)
    {
        m_transactionIDs.removeOne(telegramID);
    }

    return found;
}

QString OCUfan::myFilename()
{
    return (m_filepath + QString().sprintf("ocufan-%06i.csv", m_id));
}

bool OCUfan::isConfigured()
{
    bool configured = true;

    if ((m_fanAddress == -1) || (m_fanGroup == -1) || (m_busID == -1))
        configured = false;

    return configured;
}

void OCUfan::markAsOnline()
{
    // If we reach this point we are going to parse a telegram for this ffu, so mark it as online
    if (!m_actualData.online)
    {
        m_loghandler->slot_entryGone(LogEntry::Error, "OCUfan id=" + QString().setNum(m_id), "Not online.");
        m_actualData.online = true;
    }
    m_actualData.lastSeen = QDateTime::currentDateTime();
}

void OCUfan::setNmax(double maxRpm)
{
    if (maxRpm != m_speedMaxRPM)
    {
        m_speedMaxRPM = maxRpm;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

void OCUfan::setNmaxFromConfigData()
{
    if (!m_configData.valid)
        return;

    setNmax((double) m_configData.speedMax);
}

void OCUfan::processConfigData()
{
    setNmaxFromConfigData();
}

// ************************************************** Bus response handling **************************************************

void OCUfan::slot_transactionLost(quint64 id)
{
    Q_UNUSED(id)

    // If the ffu has a lost telegram, mark it as offline and increment error counter
    m_actualData.lostTelegrams++;
    if (m_actualData.online)
    {
        m_loghandler->slot_newEntry(LogEntry::Error, "OCUfan id=" + QString().setNum(m_id), "Not online.");
        m_actualData.online = false;
    }
    emit signal_FanActualDataHasChanged(m_id);
}

void OCUfan::slot_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, OCUfan::OcuFanModbusHoldingRegister reg, quint16 rawdata)
{
    Q_UNUSED(telegramID)

    if (adr != m_fanAddress)
        return;

    markAsOnline();

    switch (reg)
    {
    case OCUfan::HOLDING_REG_0000_TemperatureFFUSetpoint:
        m_actualData.temperatureSetpoint = (double)rawdata / 1000.0;
        processConfigData();
        break;
    case OCUfan::HOLDING_REG_0001_PumpSetpoint:
        m_actualData.pumpSetpoint = (double)rawdata / 1000.0;
        break;
    case OCUfan::HOLDING_REG_0002_FanRpmSetpoint:
        m_actualData.speedSetpoint = rawdata;
        break;
    case OCUfan::HOLDING_REG_0003_TemperatureControlKp:
        m_actualData.temperatureKp = (double)rawdata / 100.0;
        break;
    case OCUfan::HOLDING_REG_0004_TemperatureControlTn:
        m_actualData.temperatureTn = (double)rawdata / 10.0;
        break;
    default:
        break;
    }
}

void OCUfan::slot_receivedInputRegisterData(quint64 telegramID, quint16 adr, OcuFanModbusInputRegister reg, quint16 rawdata)
{
    Q_UNUSED(telegramID)

    if (adr != m_fanAddress)
        return;

    markAsOnline();

    switch (reg)
    {
    case OCUfan::INPUT_REG_0000_TemperatureFFURead:
        m_actualData.temperatureRead = (double)rawdata / 1000.0;
        emit signal_FanActualDataHasChanged(m_id);          // Temperature is the last data we get from automatic query, so signal new data now
        break;
    case OCUfan::INPUT_REG_0001_CoolingValveSetpoint:
        m_actualData.coolingValveSet = (double)rawdata / 10000.0;
        break;
    case OCUfan::INPUT_REG_0002_CoolingValveRead:
        m_actualData.coolingValveRead = (double)rawdata / 10000.0;
        break;
    case OCUfan::INPUT_REG_0003_FanErrorCode:
        if (rawdata == 0)
        {
            m_actualData.statusString = "healthy";
            m_loghandler->slot_entryGone(LogEntry::Error, "AuxFan id=" + QString().setNum(m_id), "Status error present.");
        }
        else
        {
            m_actualData.statusString = "problem";
            m_loghandler->slot_newEntry(LogEntry::Error, "AuxFan id=" + QString().setNum(m_id), "Status error present.");
        }
        break;
    case OCUfan::INPUT_REG_0004_FanRpmRead:
        m_actualData.speedReading = rawdata;
        break;
    case OCUfan::INPUT_REG_0005_FanPowerRead:
        m_actualData.fanPower = rawdata;
        break;
    default:
        break;
    }
}

void OCUfan::slot_wroteHoldingRegisterData(quint64 telegramID)
{
    Q_UNUSED(telegramID)

    markAsOnline();
}

void OCUfan::slot_save()
{
    save();
}
