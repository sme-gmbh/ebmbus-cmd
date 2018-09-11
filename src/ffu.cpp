#include "ffu.h"

#include <QFile>
#include <QString>
#include <QStringList>
#include <QDir>

FFU::FFU(QObject *parent, EbmBusSystem* ebmbusSystem) : QObject(parent)
{
    m_ebmbusSystem = ebmbusSystem;

    m_dataChanged = false;
    setAutoSave(true);

    m_id = -1;
    m_setpointSpeedRaw = 170;
    m_speedMaxRPM = 1000.0;     // Just some initial data, read back real values later from EEPROM
    m_busID = -1;
    m_unit = -1;
    m_fanAddress = -1;   // Invalid Address
    m_fanGroup = -1;     // Invalid Address

    m_remoteControlled = true;

    m_actualData.online = false;
    m_actualData.lostTelegrams = 0;
    m_actualData.lastSeen = QDateTime();
    m_actualData.speedSettingLostCount = 0;
    m_actualData.speedReading = 0;
    m_actualData.speedSetpoint = 0;
    m_actualData.statusRaw_LSB = 0;
    m_actualData.statusRaw_MSB = 0;
    m_actualData.statusString_LSB = QString();
    m_actualData.statusString_MSB = QString();
    m_actualData.warnings = 0;
    m_actualData.dcCurrent = 0.0;
    m_actualData.dcVoltage = 0.0;
    m_actualData.temperatureOfPowerModule = 0;

    m_configData.valid = false;
    m_configData.speedMax = 0;
    m_configData.manufacturingDateCode_Day = 0;
    m_configData.manufacturingDateCode_Month = 0;
    m_configData.manufacturingDateCode_Year = 0;
    m_configData.serialNumber = 0;
    m_configData.referenceDClinkVoltage = 0;
    m_configData.referenceDClinkCurrent = 0;
}

FFU::~FFU()
{

}

int FFU::getId() const
{
    return m_id;
}

void FFU::setId(int id)
{
    if (id != m_id)
    {
        m_id = id;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

void FFU::setSpeed(int rpm)
{
    int setpointSpeedRaw = this->rpmToRawSpeed(rpm);
    if (m_setpointSpeedRaw != setpointSpeedRaw)
    {
        setSpeedRaw(setpointSpeedRaw);
    }
}

void FFU::setSpeedRaw(int value, bool refreshOnly)
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
            EbmBus* bus = m_ebmbusSystem->getBusByID(m_busID);
            if (bus == NULL)
                return;     // Drop requests for non existing bus ids

            m_transactionIDs.append(bus->setSpeedSetpoint(m_fanAddress, m_fanGroup, m_setpointSpeedRaw));
        }
        else
        {
            // Some error handling here!
        }
    }
}

void FFU::setNmax(int maxRpm)
{
    if (maxRpm != m_speedMaxRPM)
    {
        m_speedMaxRPM = maxRpm;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

void FFU::setNmaxFromConfigData()
{
    if (!m_configData.valid)
        return;

    int maxRpm = 1.875e9 / ((double) m_configData.speedMax);
    setNmax(maxRpm);
}

void FFU::processConfigData()
{
    setNmaxFromConfigData();
}

int FFU::getSpeedSetpoint()
{
    return (rawSpeedToRPM(m_setpointSpeedRaw));
}

int FFU::getSpeedSetpointRaw()
{
    return m_setpointSpeedRaw;
}

double FFU::rawSpeedToRPM(int rawSpeed)
{
    return ((double)rawSpeed / 250.0 * m_speedMaxRPM);
}

int FFU::rpmToRawSpeed(double rpm)
{
    return (int)(rpm / m_speedMaxRPM * 250);
}

QString FFU::getData(QString key)
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
    else if (key == "unit")
    {
        return (QString().setNum(getUnit()));
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
    else if (key == "statusRaw_LSB")
    {
        return QString().sprintf("%02x", m_actualData.statusRaw_LSB);
    }
    else if (key == "statusRaw_MSB")
    {
        return QString().sprintf("%02x", m_actualData.statusRaw_MSB);
    }
    else if (key == "statusString")
    {
        if ((m_actualData.statusRaw_LSB == 0x00) && (m_actualData.statusRaw_MSB == 0x00))
            return "healthy";
        else
            return (m_actualData.statusString_LSB + " " + m_actualData.statusString_MSB).toUtf8().trimmed().toPercentEncoding();
    }
    else if (key == "warnings")
    {
        return QString().sprintf("%02x", m_actualData.warnings);
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

    return "Error[FFU]: Key " + key + " not available";
}

void FFU::setData(QString key, QString value)
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
    else if(key == "unit")
    {
        setUnit(value.toInt());
    }
    else if (key == "fanAddress")
    {
        setFanAddress(value.toInt());
    }
    else if (key == "fanGroup")
    {
        setFanGroup(value.toInt());
    }
//    else if (key == "nmax")
//    {
//        setNmax(value.toInt());
//    } // Not to be used anymore, because nmax is read from EEPROM
}

void FFU::setRemoteControlled(bool remoteControlled)
{
    m_remoteControlled = remoteControlled;
    if (remoteControlled)
        setAutostart(true);
    else
        setAutostart(false);
}

bool FFU::isRemoteControlled() const
{
    return m_remoteControlled;
}

QStringList FFU::getActualKeys()
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

FFU::ActualData FFU::getActualData() const
{
    return m_actualData;
}

void FFU::requestStatus()
{
    if (!isConfigured())
        return;

    EbmBus* bus = m_ebmbusSystem->getBusByID(m_busID);
    if (bus == NULL)
        return;

    if (!m_configData.valid)
        requestConfig();

    m_transactionIDs.append(bus->getActualSpeed(m_fanAddress, m_fanGroup));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::SetPoint));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::MotorStatusLowByte));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::MotorStatusHighByte));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::Warnings));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::DCvoltage));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::DCcurrent));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::TemperatureOfPowerModule));
}

void FFU::requestConfig()
{
    if (!isConfigured())
        return;

    EbmBus* bus = m_ebmbusSystem->getBusByID(m_busID);
    if (bus == NULL)
        return;

    m_transactionIDs.append(bus->readEEPROM(m_fanAddress, m_fanGroup, EbmBusEEPROM::MaxSpeed_LSB));
    m_transactionIDs.append(bus->readEEPROM(m_fanAddress, m_fanGroup, EbmBusEEPROM::MaxSpeed_Mid));
    m_transactionIDs.append(bus->readEEPROM(m_fanAddress, m_fanGroup, EbmBusEEPROM::MaxSpeed_MSB));
    m_transactionIDs.append(bus->readEEPROM(m_fanAddress, m_fanGroup, EbmBusEEPROM::ReferenceDClinkCurrent_LSB));
    m_transactionIDs.append(bus->readEEPROM(m_fanAddress, m_fanGroup, EbmBusEEPROM::ReferenceDClinkCurrent_MSB));
    m_transactionIDs.append(bus->readEEPROM(m_fanAddress, m_fanGroup, EbmBusEEPROM::ReferenceDClinkVoltage_LSB));
    m_transactionIDs.append(bus->readEEPROM(m_fanAddress, m_fanGroup, EbmBusEEPROM::ReferenceDClinkVoltage_MSB));
}

// Make shure m_setpointSpeedRaw is set to the desired value before calling this function
void FFU::setAutostart(bool enabled)
{
    if (!isConfigured())
        return;

    EbmBus* bus = m_ebmbusSystem->getBusByID(m_busID);
    if (bus == NULL)
        return;

    if (enabled)
    {
        m_transactionIDs.append(bus->writeEEPROM(m_fanAddress, m_fanGroup, EbmBusEEPROM::OperationModes_1, 0x0b));
        m_transactionIDs.append(bus->writeEEPROM(m_fanAddress, m_fanGroup, EbmBusEEPROM::SetTargetValue, m_setpointSpeedRaw));
        m_transactionIDs.append(bus->softwareReset(m_fanAddress, m_fanGroup));
    }
    else
    {
        m_transactionIDs.append(bus->writeEEPROM(m_fanAddress, m_fanGroup, EbmBusEEPROM::OperationModes_1, 0x03));
        m_transactionIDs.append(bus->softwareReset(m_fanAddress, m_fanGroup));
        // Now autostart is disabled, so set the ffu manually to the last known speed, because otherwise ffu will stop after the reset
        m_transactionIDs.append(bus->setSpeedSetpoint(m_fanAddress, m_fanGroup, m_setpointSpeedRaw));
    }
}

void FFU::save()
{
    if (!m_dataChanged)
        return;

    QFile file(myFilename());
    if (!file.open(QIODevice::WriteOnly))
        return;

    QString wdata;

    wdata.append(QString().sprintf("id=%i ", m_id));
    wdata.append(QString().sprintf("bus=%i ", m_busID));
    wdata.append(QString().sprintf("unit=%i ", m_unit));
    wdata.append(QString().sprintf("fanAddress=%i ", m_fanAddress));
    wdata.append(QString().sprintf("fanGroup=%i ", m_fanGroup));
    wdata.append(QString().sprintf("nmax=%.2lf ", m_speedMaxRPM));      // No need to save this anymore, as it is read from EEPROM in current version of this software
    wdata.append(QString().sprintf("setpointSpeedRaw=%i ", m_setpointSpeedRaw));
    wdata.append(QString().sprintf("speedSettingLostCount=%i\n", m_actualData.speedSettingLostCount));    // Todo: decide when to save this. Do not write flash to dead!


    file.write(wdata.toUtf8());

    file.close();
}

void FFU::setFiledirectory(QString path)
{
    if (!path.endsWith("/"))
        path.append("/");
    m_filepath = path;

    QDir dir;
    dir.mkpath(path);
}

void FFU::load(QString filename)
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

        if (key == "unit")
            m_unit = value.toInt();

        if (key == "fanAddress")
            m_fanAddress = value.toInt();

        if (key == "fanGroup")
            m_fanGroup = value.toInt();

        if (key == "nmax")
            m_speedMaxRPM = value.toDouble();

        if (key == "setpointSpeedRaw")
            m_setpointSpeedRaw = value.toInt();

        if (key == "speedSettingLostCount")
        {
            m_actualData.speedSettingLostCount = value.toInt();
        }
    }

    file.close();
}

void FFU::setAutoSave(bool on)
{
    m_autosave = on;
    if (m_autosave)
        connect(this, SIGNAL(signal_needsSaving()), this, SLOT(slot_save()));
    else
        disconnect(this, SIGNAL(signal_needsSaving()), this, SLOT(slot_save()));
}

void FFU::deleteFromHdd()
{
    QFile file(myFilename());
    file.remove();
}

bool FFU::isThisYourTelegram(quint64 telegramID, bool deleteID)
{
    bool found = m_transactionIDs.contains(telegramID);

    if (found && deleteID)
    {
        m_transactionIDs.removeOne(telegramID);
    }

    return found;
}

QString FFU::myFilename()
{
    return (m_filepath + QString().sprintf("ffu-%06i.csv", m_id));
}

bool FFU::isConfigured()
{
    bool configured = true;

    if ((m_fanAddress == -1) || (m_fanGroup == -1) || (m_busID == -1))
        configured = false;

    return configured;
}

void FFU::markAsOnline()
{
    // If we reach this point we are going to parse a telegram for this ffu, so mark it as online
    m_actualData.online = true;
    m_actualData.lastSeen = QDateTime::currentDateTime();
}

void FFU::slot_save()
{
    save();
}

int FFU::getBusID() const
{
    return m_busID;
}

void FFU::setBusID(int busID)
{
    if (busID != m_busID)
    {
        m_busID = busID;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

int FFU::getUnit() const
{
    return m_unit;
}

void FFU::setUnit(int unit)
{
    if (unit != m_unit)
    {
        m_unit = unit;
        m_dataChanged = true;
        emit signal_needsSaving();

        // Temporarily set to computed addresses derived from unit number, make this more beautiful later...
        setFanAddress(2);
        setFanGroup(2 + unit);
    }
}

int FFU::getFanAddress() const
{
    return m_fanAddress;
}

void FFU::setFanAddress(int fanAddress)
{
    if (m_fanAddress != fanAddress)
    {
        m_fanAddress = fanAddress;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

int FFU::getFanGroup() const
{
    return m_fanGroup;
}

void FFU::setFanGroup(int fanGroup)
{
    if (m_fanGroup != fanGroup)
    {
        m_fanGroup = fanGroup;
        m_dataChanged = true;
        emit signal_needsSaving();
    }
}

// ************************************************** Bus response handling **************************************************

void FFU::slot_transactionLost(quint64 id)
{
    Q_UNUSED(id);

    // If the ffu has a lost telegram, mark it as offline and increment error counter
    m_actualData.lostTelegrams++;
    m_actualData.online = false;
    emit signal_FFUactualDataHasChanged(m_id);
}

void FFU::slot_simpleStatus(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, QString status)
{
    Q_UNUSED(telegramID);
    Q_UNUSED(fanAddress);
    Q_UNUSED(fanGroup);
    Q_UNUSED(status);
    // We don't request the simple status, so we don't care about that response
    markAsOnline();
}

void FFU::slot_status(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 statusAddress, QString status, quint8 rawValue)
{
    Q_UNUSED(telegramID);

    if (fanAddress != m_fanAddress)
        return;
    if (fanGroup != m_fanGroup)
        return;

    markAsOnline();

    switch (statusAddress)
    {
    case EbmBusStatus::MotorStatusLowByte:
        m_actualData.statusRaw_LSB = rawValue;
        m_actualData.statusString_LSB = status;
        break;
    case EbmBusStatus::MotorStatusHighByte:
        m_actualData.statusRaw_MSB = rawValue;
        m_actualData.statusString_MSB = status;
        break;
    case EbmBusStatus::Warnings:
        m_actualData.warnings = rawValue;
        break;
    case EbmBusStatus::DCvoltage:
        m_actualData.dcVoltage = (double)rawValue * (double)m_configData.referenceDClinkVoltage * 0.02;
        break;
    case EbmBusStatus::DCcurrent:
        m_actualData.dcCurrent = (double)rawValue * (double)m_configData.referenceDClinkCurrent * 0.002;
        break;
    case EbmBusStatus::TemperatureOfPowerModule:
        m_actualData.temperatureOfPowerModule = rawValue;
        emit signal_FFUactualDataHasChanged(m_id);          // TemperatureOfPowerModule is the last data we get from automatic query, so signal new data now
        break;
    case EbmBusStatus::SetPoint:
        m_actualData.speedSetpoint = rawValue;
        // If the setpoint in the ffu does not match the setpoint in the controller, write the setpoint from the controller
        // to the ffu.
        if ((m_actualData.speedSetpoint != m_setpointSpeedRaw) && m_remoteControlled)
        {
            if (m_actualData.speedSettingLostCount < 2000)    // EEPROM wear limiter
                setSpeedRaw(m_setpointSpeedRaw, true);
            m_actualData.speedSettingLostCount++;
        }
        break;
    case EbmBusStatus::ActualValue:
        break;
    case EbmBusStatus::ModeOfControl:
        break;
    case EbmBusStatus::DirectionOfRotation:
        break;
    case EbmBusStatus::PWMdutyCycle:
        break;
    case EbmBusStatus::SteppingSwitch_1_2:
        break;
    case EbmBusStatus::SteppingSwitch_3_4:
        break;
    case EbmBusStatus::TemperatureOfMotor:
        break;
    case EbmBusStatus::LineVoltage:
        break;
    case EbmBusStatus::LineCurrent:
        break;
    case EbmBusStatus::MaxVolumetricFlowRate:
        break;
    case EbmBusStatus::MinVolumericFlowRate:
        break;
    case EbmBusStatus::MaxPressure:
        break;
    case EbmBusStatus::MinPressure:
        break;
    case EbmBusStatus::ElectronicBoxTemperature:
        break;
    case EbmBusStatus::EEPROMchecksumLSB:
        break;
    case EbmBusStatus::EEPROMchecksumMSB:
        break;
    }
}

void FFU::slot_actualSpeed(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, quint8 actualRawSpeed)
{
    Q_UNUSED(telegramID);

    if (fanAddress != m_fanAddress)
        return;
    if (fanGroup != m_fanGroup)
        return;

    markAsOnline();

    m_actualData.speedReading = actualRawSpeed;
    //emit signal_FFUactualDataHasChanged(m_id);
}

void FFU::slot_setPointHasBeenSet(quint64 telegramID, quint8 fanAddress, quint8 fanGroup)
{
    Q_UNUSED(telegramID);

    if (fanAddress != m_fanAddress)
        return;
    if (fanGroup != m_fanGroup)
        return;

    markAsOnline();

    // At the moment we dont do anything with that information.
    // Setpoint has been set - so far so good.
}

void FFU::slot_EEPROMhasBeenWritten(quint64 telegramID, quint8 fanAddress, quint8 fanGroup)
{
    Q_UNUSED(telegramID);

    if (fanAddress != m_fanAddress)
        return;
    if (fanGroup != m_fanGroup)
        return;

    markAsOnline();

    // At the moment we dont do anything with that information.
    // EEPROM has been written - so far so good.
}

void FFU::slot_EEPROMdata(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 dataByte)
{
    Q_UNUSED(telegramID);

    if (fanAddress != m_fanAddress)
        return;
    if (fanGroup != m_fanGroup)
        return;

    markAsOnline();

    switch(eepromAddress)
    {
    case EbmBusEEPROM::FanGroupAddress:
        break;
    case EbmBusEEPROM::FanAddress:
        break;
    case EbmBusEEPROM::OperationModes_1:
        break;
    case EbmBusEEPROM::SetTargetValue:
        break;

    case EbmBusEEPROM::Controller_P_factor:
        break;
    case EbmBusEEPROM::Controller_I_factor:
        break;
    case EbmBusEEPROM::Controller_D_factor:
        break;
    case EbmBusEEPROM::MaxSpeed_MSB:
        m_configData.speedMax &= 0x00FFFF;
        m_configData.speedMax |= (dataByte << 16);
        break;
    case EbmBusEEPROM::MaxSpeed_Mid:
        m_configData.speedMax &= 0xFF00FF;
        m_configData.speedMax |= (dataByte << 8);
        break;
    case EbmBusEEPROM::MaxSpeed_LSB:
        m_configData.speedMax &= 0xFFFF00;
        m_configData.speedMax |= (dataByte << 0);
        break;
    case EbmBusEEPROM::DutyCycleMax:
        break;
    case EbmBusEEPROM::DutyCycleMin:
        break;
    case EbmBusEEPROM::DutyCycleStart:
        break;
    case EbmBusEEPROM::TargetValue_0:
        break;
    case EbmBusEEPROM::TargetValue_1:
        break;

    case EbmBusEEPROM::OperationModes_2:
        break;
    case EbmBusEEPROM::RatingFactor:
        break;

        // Sensor values coded in IEEE754
    case EbmBusEEPROM::SensorValueMin_Byte_1:
        break;
    case EbmBusEEPROM::SensorValueMin_Byte_2:
        break;
    case EbmBusEEPROM::SensorValueMin_Byte_3:
        break;
    case EbmBusEEPROM::SensorValueMin_Byte_4:
        break;
    case EbmBusEEPROM::SensorValueMax_Byte_1:
        break;
    case EbmBusEEPROM::SensorValueMax_Byte_2:
        break;
    case EbmBusEEPROM::SensorValueMax_Byte_3:
        break;
    case EbmBusEEPROM::SensorValueMax_Byte_4:
        break;

    case EbmBusEEPROM::PhysicalUnitOfMeasuredQuantity_Char_01:
        break;
    case EbmBusEEPROM::PhysicalUnitOfMeasuredQuantity_Char_02:
        break;
    case EbmBusEEPROM::PhysicalUnitOfMeasuredQuantity_Char_03:
        break;
    case EbmBusEEPROM::PhysicalUnitOfMeasuredQuantity_Char_04:
        break;
    case EbmBusEEPROM::PhysicalUnitOfMeasuredQuantity_Char_05:
        break;
    case EbmBusEEPROM::PhysicalUnitOfMeasuredQuantity_Char_06:
        break;
    case EbmBusEEPROM::PhysicalUnitOfMeasuredQuantity_Char_07:
        break;
    case EbmBusEEPROM::PhysicalUnitOfMeasuredQuantity_Char_08:
        break;
    case EbmBusEEPROM::PhysicalUnitOfMeasuredQuantity_Char_09:
        break;
    case EbmBusEEPROM::PhysicalUnitOfMeasuredQuantity_Char_10:
        break;

        // Conversion factor is coded in IEEE754
    case EbmBusEEPROM::ConversionFactor_Byte_1:
        break;
    case EbmBusEEPROM::ConversionFactor_Byte_2:
        break;
    case EbmBusEEPROM::ConversionFactor_Byte_3:
        break;
    case EbmBusEEPROM::ConversionFactor_Byte_4:
        break;

    case EbmBusEEPROM::EEPROMstatus:
        break;
    case EbmBusEEPROM::MotorDesign_NumberOfPoles:
        break;
    case EbmBusEEPROM::MotorDesign_Function:
        break;
    case EbmBusEEPROM::DutyCycleMaxAdmissible:
        break;
    case EbmBusEEPROM::DutyCycleMinAdmissible:
        break;
    case EbmBusEEPROM::Identification:
        break;

    case EbmBusEEPROM::FailureDisplay_N0_HighByte:
        break;
    case EbmBusEEPROM::FailureDisplay_N1_HighByte:
        break;
    case EbmBusEEPROM::FailureDisplay_N2_HighByte:
        break;
    case EbmBusEEPROM::FailureDisplay_N0_LowByte:
        break;
    case EbmBusEEPROM::FailureDisplay_N1_LowByte:
        break;
    case EbmBusEEPROM::FailureDisplay_N2_LowByte:
        break;

    case EbmBusEEPROM::OperationHours_MSB:
        break;
    case EbmBusEEPROM::OperationHours_LSB:
        break;

    case EbmBusEEPROM::ManufacturingDateCode_Day:
         m_configData.manufacturingDateCode_Day = dataByte;
        break;
    case EbmBusEEPROM::ManufacturingDateCode_Month:
        m_configData.manufacturingDateCode_Month = dataByte;
        break;
    case EbmBusEEPROM::ManufacturingDateCode_Year:
        m_configData.manufacturingDateCode_Year = dataByte;
        break;

    case EbmBusEEPROM::SerialNumber_Byte_2:
        m_configData.serialNumber &= 0x00FFFF;
        m_configData.serialNumber |= (dataByte << 16);
        break;
    case EbmBusEEPROM::SerialNumber_Byte_1:
        m_configData.serialNumber &= 0xFF00FF;
        m_configData.serialNumber |= (dataByte << 8);
        break;
    case EbmBusEEPROM::SerialNumber_Byte_0:
        m_configData.serialNumber &= 0xFFFF00;
        m_configData.serialNumber |= (dataByte << 0);
        break;

    case EbmBusEEPROM::DClinkCurrentMax:
        break;
    case EbmBusEEPROM::AmbientTemperatureMax:
        break;
    case EbmBusEEPROM::RampUpAccelerationRate:
        break;
    case EbmBusEEPROM::RampDownDecelerationRate:
        break;
    case EbmBusEEPROM::DClinkVoltageMin:
        break;
    case EbmBusEEPROM::LineVoltageMin:
        break;
    case EbmBusEEPROM::DCIrelais:
        break;

    case EbmBusEEPROM::ReferenceDClinkVoltage_MSB:
        m_configData.referenceDClinkVoltage &= 0x00FF;
        m_configData.referenceDClinkVoltage |= (dataByte << 8);
        m_configData.valid = true;  // Set the configData valid as ReferenceDClinkVoltage_MSB is the last value requested at startup
        processConfigData();
        break;
    case EbmBusEEPROM::ReferenceDClinkVoltage_LSB:
        m_configData.referenceDClinkVoltage &= 0xFF00;
        m_configData.referenceDClinkVoltage |= (dataByte << 0);
        break;
    case EbmBusEEPROM::ReferenceDClinkCurrent_MSB:
        m_configData.referenceDClinkCurrent &= 0x00FF;
        m_configData.referenceDClinkCurrent |= (dataByte << 8);
        break;
    case EbmBusEEPROM::ReferenceDClinkCurrent_LSB:
        m_configData.referenceDClinkCurrent &= 0xFF00;
        m_configData.referenceDClinkCurrent |= (dataByte << 0);
        break;
    case EbmBusEEPROM::ReferenceAClineVoltage_MSB:
        break;
    case EbmBusEEPROM::ReferenceAClineVoltage_LSB:
        break;
    case EbmBusEEPROM::ReferenceAClineCurrent_MSB:
        break;
    case EbmBusEEPROM::ReferenceAClineCurrent_LSB:
        break;

    case EbmBusEEPROM::ActualOperationMode:
        break;
    case EbmBusEEPROM::ActualMaxPWMdutyCycle:
        break;
    case EbmBusEEPROM::ActualMinPWMdutyCycle:
        break;
    case EbmBusEEPROM::ActualTargetValue:
        break;
    case EbmBusEEPROM::ActualSensorValue:
        break;
    }
}
