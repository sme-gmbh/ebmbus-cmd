#ifndef FFUDATABASE_H
#define FFUDATABASE_H

#include <QObject>
#include <QList>
#include <QVariant>
#include "ffu.h"

class FFUdatabase : public QObject
{
    Q_OBJECT
public:
    explicit FFUdatabase(QObject *parent = 0);

    void loadFromHdd();
    void saveToHdd();
    QString addFFU(int id);
    QString deleteFFU(int id);

    QList<FFU*> getFFUs();
    FFU* getFFUbyID(int id);

    QString getFFUdata(int id, QString key);
    QString setFFUdata(int id, QString key, QString value);

private:
    QList<FFU*> m_ffus;

signals:

public slots:
};

#endif // FFUDATABASE_H
