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

#include "auxfan.h"

#include <QFile>
#include <QString>
#include <QStringList>
#include <QDir>

AuxFan::AuxFan(QObject *parent, EbmModbusSystem *ebmModbusSystem, Loghandler *loghandler) : QObject(parent)
{
    m_ebmModbusSystem = ebmModbusSystem;
    m_loghandler = loghandler;

    m_dataChanged = false;
    setAutoSave(true);

    m_id = -1;
    m_setpointSpeedRaw = 24928;
    m_speedMaxRPM = 770.0;     // Just some initial data, read back real values later from EEPROM
    m_busID = -1;
    m_fanAddress = -1;   // Invalid Address

    m_remoteControlled = true;

    m_actualData.online = false;
    m_actualData.lostTelegrams = 0;
    m_actualData.lastSeen = QDateTime();
    m_actualData.speedSettingLostCount = 0;
    m_actualData.speedReading = 0;
    m_actualData.speedSetpoint = 0;
    m_actualData.statusRaw = 0;
    m_actualData.statusString = QString();
    m_actualData.warnings = 0;
    m_actualData.dcCurrent = 0.0;
    m_actualData.dcVoltage = 0.0;
    m_actualData.dcPower = 0.0;
    m_actualData.temperatureOfPowerModule = 0;

    m_configData.valid = false;
    m_configData.speedMax = 0;
    m_configData.referenceDClinkVoltage = 0;
    m_configData.referenceDClinkCurrent = 0;
}

AuxFan::~AuxFan()
{

}

int AuxFan::getId() const
{
    return m_id;
}

void AuxFan::setId(int id)
{
    if (id != m_id)
    {
        m_id = id;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

int AuxFan::getBusID() const
{
    return m_busID;
}

void AuxFan::setBusID(int busID)
{
    if (busID != m_busID)
    {
        m_busID = busID;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

int AuxFan::getFanAddress() const
{
    return m_fanAddress;
}

void AuxFan::setFanAddress(int fanAddress)
{
    if (m_fanAddress != fanAddress)
    {
        m_fanAddress = fanAddress;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

void AuxFan::setSpeed(double rpm)
{
    int setpointSpeedRaw = this->rpmToRawSpeed(rpm);
    if (m_setpointSpeedRaw != setpointSpeedRaw)
    {
        setSpeedRaw(setpointSpeedRaw);
    }
}

void AuxFan::setSpeedRaw(int value, bool refreshOnly)
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
            EbmModbus* bus = m_ebmModbusSystem->getBusByID(m_busID);
            if (bus == NULL)
                return;     // Drop requests for non existing bus ids

            m_transactionIDs.append(m_ebmModbusSystem->writeHoldingRegister(m_busID, m_fanAddress, EbmModbus::HOLDING_REG_D001_DefaultSetValue, m_setpointSpeedRaw));
//            m_transactionIDs.append(bus->setSpeedSetpoint(m_fanAddress, m_setpointSpeedRaw));
        }
        else
        {
            // Some error handling here!
        }
    }
}

int AuxFan::getSpeedSetpoint()
{
    return (rawSpeedToRPM(m_setpointSpeedRaw));
}

int AuxFan::getSpeedSetpointRaw()
{
    return m_setpointSpeedRaw;
}

double AuxFan::rawSpeedToRPM(int rawSpeed)
{
    return ((double)rawSpeed / 64000.0 * m_speedMaxRPM);
}

int AuxFan::rpmToRawSpeed(double rpm)
{
    return (int)(rpm / m_speedMaxRPM * 64000.0);
}

QString AuxFan::getData(QString key)
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
    else if (key == "statusRaw_LSB")
    {
        return QString().sprintf("%02x", m_actualData.statusRaw & 0x00FF);
    }
    else if (key == "statusRaw_MSB")
    {
        return QString().sprintf("%02x", m_actualData.statusRaw >> 8);
    }
    else if (key == "statusString")
    {
        if ((m_actualData.statusRaw == 0x0000))
            return "healthy";
        else
            return m_actualData.statusString.toUtf8().trimmed().toPercentEncoding();
    }
    else if (key == "warnings")
    {
        return QString().sprintf("%04x", m_actualData.warnings);
    }
    else if (key == "dcVoltage")
    {
        return QString().sprintf("%.1lf", m_actualData.dcVoltage);
    }
    else if (key == "dcCurrent")
    {
        return QString().sprintf("%.3lf", m_actualData.dcCurrent);
    }
    else if (key == "temperatureOfPowerModule")
    {
        return QString().sprintf("%i", m_actualData.temperatureOfPowerModule);
    }

    return "Error[AuxFan]: Key " + key + " not available";
}

void AuxFan::setData(QString key, QString value)
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
}

void AuxFan::setRemoteControlled(bool remoteControlled)
{
    m_remoteControlled = remoteControlled;
    if (remoteControlled)
        setAutostart(true);
//    else  // Do not disable autostart here, because this leads to a drop in RPM when software reset occures. Do it as broadcast as soon as user selects manual speed.
//        setAutostart(false);
}

bool AuxFan::isRemoteControlled() const
{
    return m_remoteControlled;
}

QStringList AuxFan::getActualKeys()
{
    QStringList keys;

    keys += "online";
    keys += "lostTelegrams";
    keys += "lastSeen";
    keys += "speedSettingLostCount";
    keys += "speedReading";
    keys += "speedSetpoint";
    keys += "statusRaw_LSB";
    keys += "statusRaw_MSB";
    keys += "statusString";
    keys += "warnings";
    keys += "dcVoltage";
    keys += "dcCurrent";
    keys += "temperatureOfPowerModule";

    return keys;
}

AuxFan::ActualData AuxFan::getActualData() const
{
    return m_actualData;
}

void AuxFan::requestStatus()
{
    if (!isConfigured())
    {
        return;
    }

    EbmModbus* bus = m_ebmModbusSystem->getBusByID(m_busID);
    if (bus == NULL)
    {
        return;
    }

    if (!m_configData.valid)
        requestConfig();

    m_transactionIDs.append(m_ebmModbusSystem->readInputRegister(m_busID, m_fanAddress, EbmModbus::INPUT_REG_D010_ActualSpeed));
//    m_transactionIDs.append(m_ebmModbusSystem->readInputRegister(m_busID, m_fanAddress, EbmModbus::INPUT_REG_D021_CurrentPower));
    m_transactionIDs.append(m_ebmModbusSystem->readInputRegister(m_busID, m_fanAddress, EbmModbus::INPUT_REG_D013_DClinkVoltage));
    m_transactionIDs.append(m_ebmModbusSystem->readInputRegister(m_busID, m_fanAddress, EbmModbus::INPUT_REG_D014_DClinkCurrent));
    m_transactionIDs.append(m_ebmModbusSystem->readInputRegister(m_busID, m_fanAddress, EbmModbus::INPUT_REG_D01A_CurrentSetValue));
    m_transactionIDs.append(m_ebmModbusSystem->readInputRegister(m_busID, m_fanAddress, EbmModbus::INPUT_REG_D011_MotorStatus));
    m_transactionIDs.append(m_ebmModbusSystem->readInputRegister(m_busID, m_fanAddress, EbmModbus::INPUT_REG_D012_Warning));
    m_transactionIDs.append(m_ebmModbusSystem->readInputRegister(m_busID, m_fanAddress, EbmModbus::INPUT_REG_D015_ModuleTemperature));
}

void AuxFan::requestConfig()
{
    if (!isConfigured())
        return;

    EbmModbus* bus = m_ebmModbusSystem->getBusByID(m_busID);
    if (bus == NULL)
        return;

    m_transactionIDs.append(m_ebmModbusSystem->readHoldingRegister(m_busID, m_fanAddress, EbmModbus::HOLDING_REG_D119_MaximumSpeed));
    m_transactionIDs.append(m_ebmModbusSystem->readHoldingRegister(m_busID, m_fanAddress, EbmModbus::HOLDING_REG_D1A1_ReferenceValueOfDClinkCurrent));
    // Last Request DC link voltage triggers processing of config data if response telegram is processed
    m_transactionIDs.append(m_ebmModbusSystem->readHoldingRegister(m_busID, m_fanAddress, EbmModbus::HOLDING_REG_D1A0_ReferenceValueOfDClinkVoltage));
}

void AuxFan::setAutostart(bool enabled)
{
    Q_UNUSED(enabled);
// TBD
}

void AuxFan::save()
{
    if (!m_dataChanged)
        return;

    QFile file(myFilename());
    if (!file.open(QIODevice::WriteOnly))
        return;

    QString wdata;

    wdata.append(QString().sprintf("id=%i ", m_id));
    wdata.append(QString().sprintf("bus=%i ", m_busID));
    wdata.append(QString().sprintf("fanAddress=%i ", m_fanAddress));
    wdata.append(QString().sprintf("setpointSpeedRaw=%i ", m_setpointSpeedRaw));
    wdata.append(QString().sprintf("speedSettingLostCount=%i\n", m_actualData.speedSettingLostCount));    // Todo: decide when to save this. Do not write flash to dead!

    file.write(wdata.toUtf8());

    file.close();
}

void AuxFan::setFiledirectory(QString path)
{
    if (!path.endsWith("/"))
        path.append("/");
    m_filepath = path;

    QDir dir;
    dir.mkpath(path);
}

void AuxFan::load(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
        return;

    QString rdata = QString().fromUtf8(file.readLine());

    QStringList dataList = rdata.split(" ");
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

        if (key == "setpointSpeedRaw")
            m_setpointSpeedRaw = value.toInt();

        if (key == "speedSettingLostCount")
        {
            m_actualData.speedSettingLostCount = value.toInt();
        }
    }

    file.close();
}

void AuxFan::setAutoSave(bool on)
{
    m_autosave = on;
    if (m_autosave)
        connect(this, SIGNAL(signal_needsSaving()), this, SLOT(slot_save()));
    else
        disconnect(this, SIGNAL(signal_needsSaving()), this, SLOT(slot_save()));
}

void AuxFan::deleteFromHdd()
{
    QFile file(myFilename());
    file.remove();
}

void AuxFan::deleteAllErrors()
{
    m_loghandler->slot_entryGone(LogEntry::Error, "AuxFan id=" + QString().setNum(m_id), "Not online.");
    m_loghandler->slot_entryGone(LogEntry::Warning, "AuxFan id=" + QString().setNum(m_id), "Warnings present.");
}

bool AuxFan::isThisYourTelegram(quint64 telegramID, bool deleteID)
{
    bool found = m_transactionIDs.contains(telegramID);

    if (found && deleteID)
    {
        m_transactionIDs.removeOne(telegramID);
    }

    return found;
}

QString AuxFan::myFilename()
{
    return (m_filepath + QString().sprintf("auxfan-%06i.csv", m_id));
}

bool AuxFan::isConfigured()
{
    bool configured = true;

    if ((m_fanAddress == -1) || (m_busID == -1))
        configured = false;

    return configured;
}

void AuxFan::markAsOnline()
{
    // If we reach this point we are going to parse a telegram for this ffu, so mark it as online
    if (!m_actualData.online)
    {
        m_loghandler->slot_entryGone(LogEntry::Error, "AuxFan id=" + QString().setNum(m_id), "Not online.");
        m_actualData.online = true;
    }
    m_actualData.lastSeen = QDateTime::currentDateTime();
}

void AuxFan::setNmax(double maxRpm)
{
    if (maxRpm != m_speedMaxRPM)
    {
        m_speedMaxRPM = maxRpm;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

void AuxFan::setNmaxFromConfigData()
{
    if (!m_configData.valid)
        return;

    setNmax((double) m_configData.speedMax);
}

void AuxFan::processConfigData()
{
    setNmaxFromConfigData();
}

// ************************************************** Bus response handling **************************************************

void AuxFan::slot_transactionLost(quint64 id)
{
    Q_UNUSED(id);

    // If the ffu has a lost telegram, mark it as offline and increment error counter
    m_actualData.lostTelegrams++;
    if (m_actualData.online)
    {
        m_loghandler->slot_newEntry(LogEntry::Error, "AuxFan id=" + QString().setNum(m_id), "Not online.");
        m_actualData.online = false;
    }
    emit signal_FanActualDataHasChanged(m_id);
}

void AuxFan::slot_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg, quint16 rawdata)
{
    Q_UNUSED(telegramID);

    if (adr != m_fanAddress)
        return;

    markAsOnline();

    switch (reg)
    {
    case EbmModbus::HOLDING_REG_D103_SaveSetValue:
        break;
    case EbmModbus::HOLDING_REG_D119_MaximumSpeed:
        m_configData.speedMax = rawdata;
        break;
    case EbmModbus::HOLDING_REG_D180_OperatingHoursCounter:
        break;
    case EbmModbus::HOLDING_REG_D181_OperatingMinutesCounter:
        break;
    case EbmModbus::HOLDING_REG_D1A0_ReferenceValueOfDClinkVoltage:
        m_configData.referenceDClinkVoltage = rawdata;
        m_configData.valid = true;  // Set the configData valid as ReferenceDClinkVoltage_MSB is the last value requested at startup
        processConfigData();
        break;
    case EbmModbus::HOLDING_REG_D1A1_ReferenceValueOfDClinkCurrent:
        m_configData.referenceDClinkCurrent = rawdata;
        break;
    case EbmModbus::HOLDING_REG_D1A2_FanSerialNumber_MSB:
        break;
    case EbmModbus::HOLDING_REG_D1A3_FanSerialNumber_LSB:
        break;
    default:
    break;
    }
}

void AuxFan::slot_receivedInputRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusInputRegister reg, quint16 rawdata)
{
    Q_UNUSED(telegramID);

    if (adr != m_fanAddress)
        return;

    markAsOnline();

    switch (reg)
    {
    case EbmModbus::INPUT_REG_D010_ActualSpeed:
        m_actualData.speedReading = rawdata;
        break;
    case EbmModbus::INPUT_REG_D011_MotorStatus:
        m_actualData.statusRaw = rawdata;
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
    case EbmModbus::INPUT_REG_D012_Warning:
        m_actualData.warnings = rawdata;
        if (m_actualData.warnings != 0)
            m_loghandler->slot_newEntry(LogEntry::Warning, "AuxFan id=" + QString().setNum(m_id), "Warnings present.");
        else
            m_loghandler->slot_entryGone(LogEntry::Warning, "AuxFan id=" + QString().setNum(m_id), "Warnings present.");
        break;
    case EbmModbus::INPUT_REG_D013_DClinkVoltage:
        m_actualData.dcVoltage = (double)rawdata / 265.0 * (double)m_configData.referenceDClinkVoltage * 0.02;
        break;
    case EbmModbus::INPUT_REG_D014_DClinkCurrent:
        m_actualData.dcCurrent = (double)rawdata / 256.0 * (double)m_configData.referenceDClinkCurrent * 0.002;
        break;
    case EbmModbus::INPUT_REG_D015_ModuleTemperature:
        m_actualData.temperatureOfPowerModule = rawdata;
        emit signal_FanActualDataHasChanged(m_id);          // TemperatureOfPowerModule is the last data we get from automatic query, so signal new data now
        break;
    case EbmModbus::INPUT_REG_D01A_CurrentSetValue:
        m_actualData.speedSetpoint = rawdata;
        // If the setpoint in the ffu does not match the setpoint in the controller, write the setpoint from the controller
        // to the ffu.
        if ((m_actualData.speedSetpoint != m_setpointSpeedRaw) && m_remoteControlled)
        {
            if (m_actualData.speedSettingLostCount < 2000)    // EEPROM wear limiter
                setSpeedRaw(m_setpointSpeedRaw, true);
            m_actualData.speedSettingLostCount++;
        }
        break;
    case EbmModbus::INPUT_REG_D021_CurrentPower:
        m_actualData.dcPower = (double)rawdata / 65536 * (double)m_configData.referenceDClinkVoltage * 0.02 * (double)m_configData.referenceDClinkCurrent * 0.002;
        break;
    default:
        break;
    }
}

void AuxFan::slot_wroteHoldingRegisterData(quint64 telegramID)
{
    Q_UNUSED(telegramID);

//    if (fanAddress != m_fanAddress)
//        return;

    markAsOnline();
}

void AuxFan::slot_save()
{
    save();
}
