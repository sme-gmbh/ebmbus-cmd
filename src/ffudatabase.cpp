#include "ffudatabase.h"

FFUdatabase::FFUdatabase(QObject *parent) : QObject(parent)
{

}

void FFUdatabase::loadFromHdd()
{
    // tbd.
}

void FFUdatabase::saveToHdd()
{
    foreach (FFU* ffu, m_ffus)
    {
        // tbd.
    }
}

QString FFUdatabase::addFFU(int id)
{
    FFU* newFFU = new FFU(this);
    newFFU->setId(id);
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
        delete ffu;
        return "OK[FFUdatabase]: Removed ID " + QString().setNum(id);
    }
    return "Warning[FFUdatabase]: Unable to remove ID " + QString().setNum(id) + " from db.";
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

QString FFUdatabase::setFFUdata(int id, QString key, QString value)
{
    FFU* ffu = getFFUbyID(id);
    if (ffu == NULL)
        return "Warning[FFUdatabase]: ID " + QString().setNum(id) + " not found.";

    ffu->setData(key, value);
    return "OK[FFUdatabase]: Setting " + key + " to " + value;
}
