#include "ffudatabase.h"

FFUdatabase::FFUdatabase(QObject *parent) : QObject(parent)
{

}

void FFUdatabase::loadFromHdd()
{
    QString directory = "/tmp/openffucontrol/ffus/";
    QDirIterator iterator(directory, QStringList() << "*.csv", QDir::Files, QDirIterator::NoIteratorFlags);
    while(iterator.hasNext())
    {
        QString filepath = iterator.next();
        FFU* newFFU = new FFU(this);
        newFFU->load(filepath);
        newFFU->setFiledirectory(directory);
        m_ffus.append(newFFU);
    }
}

void FFUdatabase::saveToHdd()
{
    QString path = "/tmp/openffucontrol/ffus/";
    QDir dir;
    dir.mkpath(path);
    foreach (FFU* ffu, m_ffus)
    {
        ffu->setFiledirectory(path);
        ffu->save();
    }
}

QString FFUdatabase::addFFU(int id, int busID)
{
    FFU* newFFU = new FFU(this);
    newFFU->setFiledirectory("/tmp/openffucontrol/ffus/");
    newFFU->setAutoSave(false);
    newFFU->setId(id);
    newFFU->setBusID(busID);
    newFFU->setAutoSave(true);
    newFFU->save();
    m_ffus.append(newFFU);

    return "OK[FFUdatabase]: Added FFU ID " + QString().setNum(id);
}

QString FFUdatabase::deleteFFU(int id)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == NULL)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    bool ok = m_ffus.removeOne(ffu);
    if (ok)
    {
        ffu->deleteFromHdd();
        delete ffu;
        return "OK[FFUdatabase]: Removed ID " + QString().setNum(id);
    }
    return "Warning[FFUdatabase]: Unable to remove ID " + QString().setNum(id) + " from db.";
}

QList<FFU *> FFUdatabase::getFFUs()
{
    return m_ffus;
}

FFU *FFUdatabase::getFFUbyID(int id)
{
    foreach (FFU* ffu, m_ffus)
    {
        if (ffu->getId() == id)
            return ffu;
    }
    return NULL;    // Not found
}

QString FFUdatabase::getFFUdata(int id, QString key)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == NULL)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    return ffu->getData(key);
}

QMap<QString,QString> FFUdatabase::getFFUdata(int id, QStringList keys)
{
    QMap<QString,QString> response;

    FFU* ffu = getFFUbyID(id);
    if (ffu == NULL)
    {
        return response;
    }

    foreach (QString key, keys)
    {
        response.insert(key, ffu->getData(key));
    }

    return response;
}

QString FFUdatabase::setFFUdata(int id, QString key, QString value)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == NULL)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    ffu->setData(key, value);
    return "OK[FFUdatabase]: Setting " + key + " to " + value;
}

QString FFUdatabase::setFFUdata(int id, QMap<QString, QString> dataMap)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == NULL)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    QString dataString;

    foreach(QString key, dataMap.keys())
    {
        QString value = dataMap.value(key);
        ffu->setData(key, value);

        dataString.append(" " + key + ":" + value);
    }

    return "OK[FFUdatabase]: Setting data:" + dataString;
}
