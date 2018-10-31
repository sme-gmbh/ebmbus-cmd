#include "auxfandatabase.h"

AuxFanDatabase::AuxFanDatabase(QObject *parent,  EbmModbusSystem *ebmModbusSystem, Loghandler *loghandler) : QObject(parent)
{
    m_ebmModbusSystem = ebmModbusSystem;
}

void AuxFanDatabase::loadFromHdd()
{

}

void AuxFanDatabase::saveToHdd()
{

}

QString AuxFanDatabase::addAuxFan(int id, int busID, int unit, int fanAddress)
{

}

QString AuxFanDatabase::deleteAuxFan(int id)
{

}

QList<AuxFan *> AuxFanDatabase::getAuxFans(int busNr)
{

}

AuxFan *AuxFanDatabase::getAuxFansByID(int id)
{

}
