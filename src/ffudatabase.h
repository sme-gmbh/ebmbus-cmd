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

class FFUdatabase : public QObject
{
    Q_OBJECT
public:
    explicit FFUdatabase(QObject *parent = 0);

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

private:
    QList<FFU*> m_ffus;

signals:

public slots:
};

#endif // FFUDATABASE_H
