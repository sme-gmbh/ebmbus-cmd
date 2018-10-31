#ifndef AUXFANDATABASE_H
#define AUXFANDATABASE_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QMap>
#include <QDir>
#include <QDirIterator>
#include <QTimer>
#include <QMap>
#include "ebmmodbussystem.h"
#include "ebmmodbussystem.h"
#include "loghandler.h"
#include "auxfan.h"

// AuxFans are managed via Modbus

class AuxFanDatabase : public QObject
{
    Q_OBJECT
public:
    explicit AuxFanDatabase(QObject *parent, EbmModbusSystem *ebmModbusSystem, Loghandler *loghandler);

    void loadFromHdd();
    void saveToHdd();
    QString addAuxFan(int id, int busID, int unit = -1, int fanAddress = -1);
    QString deleteAuxFan(int id);

    QList<AuxFan*> getAuxFans(int busNr = -1);    // If busNr is specified only fans of that bus are returned
    AuxFan* getAuxFansByID(int id);

private:
    EbmModbusSystem* m_ebmModbusSystem;

signals:

public slots:
};

#endif // AUXFANDATABASE_H
