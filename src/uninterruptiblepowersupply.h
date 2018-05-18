#ifndef UNINTERRUPTIBLEPOWERSUPPLY_H
#define UNINTERRUPTIBLEPOWERSUPPLY_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include "revpidio.h"

class UninterruptiblePowerSupply : public QObject
{
    Q_OBJECT
public:
    explicit UninterruptiblePowerSupply(QObject *parent, RevPiDIO *io, int address_mainswitch);

    void setShutdownTimeout(int milliseconds);
    void setMainswitchDelay(int milliseconds);

    void startPSUshutdownTimer();

private:
    RevPiDIO* m_io;
    int m_address_mainswitch;
    QTimer m_timer;
    QDateTime m_dateTime_mainSwitchOff;
    QDateTime m_dateTime_powerfail;
    int m_shutdownTimeout;
    int m_mainswitchDelay;
    bool m_mainswitchState;
    bool m_powerstate;
    bool m_old_mainswitchState;
    bool m_mainswitchOffSignaled;

signals:
    void signal_mainswitchOff();

public slots:

private slots:
    void slot_timer_fired();
};

#endif // UNINTERRUPTIBLEPOWERSUPPLY_H
