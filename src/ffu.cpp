#include "ffu.h"

FFU::FFU(QObject *parent) : QObject(parent)
{

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
    m_id = id;
}

void FFU::setSpeed(int rpm)
{
    m_speedRaw = this->rpmToRawSpeed(rpm);
}

void FFU::setSpeedRaw(int value)
{
    m_speedRaw = value;
}

void FFU::setMaxRPM(int maxRpm)
{
    m_speedMaxRPM = maxRpm;
}

int FFU::getSpeed()
{
    return (rawSpeedToRPM(m_speedRaw));
}

int FFU::getSpeedRaw()
{
    return m_speedRaw;
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
    if (key == "rpm")
    {
        return (QString().setNum(getSpeed()));
    }
    else if (key == "rawspeed")
    {
        return (QString().setNum(getSpeedRaw()));
    }
    else if (key == "busID")
    {
        return (QString().setNum(getBusID()));
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
}

int FFU::getBusID() const
{
    return m_busID;
}

void FFU::setBusID(int busID)
{
    m_busID = busID;
}


