#ifndef FFU_H
#define FFU_H

#include <QObject>

class FFU : public QObject
{
    Q_OBJECT
public:
    explicit FFU(QObject *parent = 0);
    ~FFU();

    int getId() const;
    void setId(int id);

    int getBusID() const;
    void setBusID(int busID);

    void setSpeed(int rpm);
    void setSpeedRaw(int value);
    void setMaxRPM(int maxRpm);

    int getSpeed();
    int getSpeedRaw();

    double rawSpeedToRPM(int rawSpeed);
    int rpmToRawSpeed(double rpm);

    QString getData(QString key);
    void setData(QString key, QString value);


private:
    int m_id;
    int m_speedRaw;
    double m_speedMaxRPM;
    int m_busID;

signals:

public slots:
};

#endif // FFU_H
