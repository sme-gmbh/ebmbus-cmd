#include "ffu.h"

#include <QFile>
#include <QString>
#include <QStringList>
#include <QDir>

FFU::FFU(QObject *parent) : QObject(parent)
{
    m_dataChanged = false;
    setAutoSave(true);

    m_id = -1;
    m_setpointSpeedRaw = 0;
    m_speedMaxRPM = 0.0;
    m_busID = -1;
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
        m_dataChanged = true;
        emit signal_needsSaving();
    }

    m_id = id;
}

void FFU::setSpeed(int rpm)
{
    int setpointSpeedRaw = this->rpmToRawSpeed(rpm);
    if (m_setpointSpeedRaw != setpointSpeedRaw)
    {
        m_dataChanged = true;
        emit signal_needsSaving();
    }
    m_setpointSpeedRaw = setpointSpeedRaw;
}

void FFU::setSpeedRaw(int value)
{
    if (value != m_setpointSpeedRaw)
    {
        m_dataChanged = true;
        emit signal_needsSaving();
    }
    m_setpointSpeedRaw = value;
}

void FFU::setMaxRPM(int maxRpm)
{
    if (maxRpm != m_speedMaxRPM)
    {
        m_dataChanged = true;
        emit signal_needsSaving();
    }
    m_speedMaxRPM = maxRpm;
}

int FFU::getSpeed()
{
    return (rawSpeedToRPM(m_setpointSpeedRaw));
}

int FFU::getSpeedRaw()
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

void FFU::save()
{
    if (!m_dataChanged)
        return;

    QFile file(myFilename());
    if (!file.open(QIODevice::WriteOnly))
        return;

    QString wdata;

    wdata.sprintf("id=%i bus=%i speedMaxRPM=%8.2lf setpointSpeedRaw=%i\n", m_id, m_busID, m_speedMaxRPM, m_setpointSpeedRaw);

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

        if (key == "speedMaxRPM")
            m_speedMaxRPM = value.toDouble();

        if (key == "setpointSpeedRaw")
            m_setpointSpeedRaw = value.toInt();
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

QString FFU::myFilename()
{
    return (m_filepath + QString().sprintf("ffu-%06i.csv", m_id));
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
        m_dataChanged = true;
        emit signal_needsSaving();
    }
    m_busID = busID;
}


