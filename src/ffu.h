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

    void save();
    void setFiledirectory(QString path);
    void load(QString filename);

    void setAutoSave(bool on);

    void deleteFromHdd();


private:
    int m_id;
    int m_setpointSpeedRaw;
    double m_speedMaxRPM;
    int m_busID;

    bool m_dataChanged;
    bool m_autosave;
    QString m_filepath;

    QString myFilename();

signals:
    void signal_needsSaving();

public slots:

private slots:
    void slot_save();
};

#endif // FFU_H
