#ifndef FFUDATABASE_H
#define FFUDATABASE_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QMap>
#include <QDir>
#include <QDirIterator>
#include "ffu.h"
#include "libebmbus/ebmbus.h"

class FFUdatabase : public QObject
{
    Q_OBJECT
public:
    explicit FFUdatabase(QObject *parent, QList<EbmBus*>* ebmbuslist);

    void loadFromHdd();
    void saveToHdd();
    QString addFFU(int id, int busID);
    QString deleteFFU(int id);

    QList<FFU*> getFFUs();
    FFU* getFFUbyID(int id);

    QString getFFUdata(int id, QString key);
    QMap<QString,QString> getFFUdata(int id, QStringList keys);
    QString setFFUdata(int id, QString key, QString value);
    QString setFFUdata(int id, QMap<QString,QString> dataMap);

    QString startDCIaddressing(int busID, QString startAddress);

private:
    QList<EbmBus*>* m_ebmbuslist;
    QList<FFU*> m_ffus;

signals:
    void signal_DCIaddressingFinished(int busID);

public slots:

private slots:
    void slot_DaisyChainAdressingFinished();
};

#endif // FFUDATABASE_H
