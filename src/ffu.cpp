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
    m_setpointSpeedRaw = 0;
    m_speedMaxRPM = 0.0;
    m_busID = -1;
    m_fanAddress = -1;   // Invalid Address
    m_fanGroup = -1;     // Invalid Address

    m_remoteControlled = true;

    m_actualData.online = false;
    m_actualData.lostTelegrams = 0;
    m_actualData.speedSettingLostCount = 0;
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

            bus->setSpeedSetpoint(m_fanAddress, m_fanGroup, m_setpointSpeedRaw);
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
    return ((double)rawSpeed / 255.0 * m_speedMaxRPM);
}

int FFU::rpmToRawSpeed(double rpm)
{
    return (int)(rpm / m_speedMaxRPM * 255);
}

QString FFU::getData(QString key)
{
    if (key == "id")
    {
        return (QString().setNum(m_id));
    }
    else if (key == "rpm")
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
    else if (key == "actual")
    {
        QString response;

        response += QString().sprintf("online=%i ", m_actualData.online);
        response += QString().sprintf("lostTelegrams=%lli ", m_actualData.lostTelegrams);
        response += "lastSeen=" + m_actualData.lastSeen.toString("yyyy.MM.dd-hh:mm:ss.zzz ");
        response += QString().sprintf("speedSettingLostCount=%i ", m_actualData.speedSettingLostCount);
        response += QString().sprintf("speedReading=%i ", m_actualData.speedReading);
        response += QString().sprintf("speedSetpoint=%i ", m_actualData.speedSetpoint);
        response += QString().sprintf("statusRaw_LSB=%02x ", m_actualData.statusRaw_LSB);
        response += QString().sprintf("statusRaw_MSB=%02x ", m_actualData.statusRaw_MSB);
        response += "statusString=" + (m_actualData.statusString_LSB + " " + m_actualData.statusString_MSB + " ");    // Todo: convert string to base64
        response += QString().sprintf("warnings=%02x ", m_actualData.warnings);
        response += QString().sprintf("dcVoltage=%i ", m_actualData.dcVoltage);
        response += QString().sprintf("dcCurrent=%i ", m_actualData.dcCurrent);
        response += QString().sprintf("temperatureOfPowerModule=%i ", m_actualData.temperatureOfPowerModule);

        return response;
    }

    return "Error[FFU]: Key " + key + " not available";
}

void FFU::setData(QString key, QString value)
{
    if (key == "rpm")
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
    else if (key == "nmax")
    {
        setNmax(value.toInt());
    }
}

void FFU::setRemoteControlled(bool remoteControlled)
{
    m_remoteControlled = remoteControlled;
}

bool FFU::isRemoteControlled() const
{
    return m_remoteControlled;
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

    m_transactionIDs.append(bus->getActualSpeed(m_fanAddress, m_fanGroup));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::SetPoint));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::MotorStatusLowByte));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::MotorStatusHighByte));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::Warnings));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::DCvoltage));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::DCcurrent));
    m_transactionIDs.append(bus->getStatus(m_fanAddress, m_fanGroup, EbmBusStatus::TemperatureOfPowerModule));
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
    wdata.append(QString().sprintf("fanAddress=%i ", m_fanAddress));
    wdata.append(QString().sprintf("fanGroup=%i ", m_fanGroup));
    wdata.append(QString().sprintf("nmax=%.2lf ", m_speedMaxRPM));
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

int FFU::getFanAddress() const
{
    return m_fanAddress;
}

void FFU::setFanAddress(int fanAddress)
{
    m_fanAddress = fanAddress;
}

int FFU::getFanGroup() const
{
    return m_fanGroup;
}

void FFU::setFanGroup(int fanGroup)
{
    m_fanGroup = fanGroup;
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
        m_actualData.dcVoltage = rawValue;
        break;
    case EbmBusStatus::DCcurrent:
        m_actualData.dcCurrent = rawValue;
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
            if (m_actualData.speedSettingLostCount < 20)    // EEPROM wear limiter
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

    markAsOnline();

    if (fanAddress != m_fanAddress)
        return;
    if (fanGroup != m_fanGroup)
        return;

    // At the moment we dont do anything with that information.
    // Setpoint has been set - so far so good.
}

void FFU::slot_EEPROMhasBeenWritten(quint64 telegramID, quint8 fanAddress, quint8 fanGroup)
{
    Q_UNUSED(telegramID);

    markAsOnline();

    if (fanAddress != m_fanAddress)
        return;
    if (fanGroup != m_fanGroup)
        return;

    // At the moment we dont do anything with that information.
    // EEPROM has been written - so far so good.
}

void FFU::slot_EEPROMdata(quint64 telegramID, quint8 fanAddress, quint8 fanGroup, EbmBusEEPROM::EEPROMaddress eepromAddress, quint8 dataByte)
{
    Q_UNUSED(telegramID);

    markAsOnline();

    if (fanAddress != m_fanAddress)
        return;
    if (fanGroup != m_fanGroup)
        return;

    // Todo: do something here with the data from the EEPROM...
}
