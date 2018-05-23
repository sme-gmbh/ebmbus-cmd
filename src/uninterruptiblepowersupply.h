#ifndef UNINTERRUPTIBLEPOWERSUPPLY_H
#define UNINTERRUPTIBLEPOWERSUPPLY_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include "revpidio.h"

#ifdef USE_LIBFTDI
#include <ftdi.h>
#elif USE_LIBFTDI1
#include <libftdi1/ftdi.h>
#endif

class UninterruptiblePowerSupply : public QObject
{
    Q_OBJECT
public:
    explicit UninterruptiblePowerSupply(QObject *parent, RevPiDIO *io, int address_mainswitch);

    void setShutdownTimeout(int milliseconds);
    void setMainswitchDelay(int milliseconds);
    void setPowerGoodDelay(int milliseconds);

private:
    RevPiDIO* m_io;
    int m_address_mainswitch;
    QTimer m_timer;
    QTimer m_shutdownTimer;
    QTimer m_powerGoodTimer;
    QDateTime m_dateTime_mainSwitchOff;
    QDateTime m_dateTime_powerfail;
    int m_shutdownTimeout;
    int m_mainswitchDelay;
    bool m_mainswitchState;
    bool m_powerstate;
    bool m_old_mainswitchState;
    bool m_mainswitchOffSignaled;

    struct ftdi_context *m_ftdi;
    QByteArray m_upsResponseBuffer;

    bool m_ups_bufferReady;     // True if ups could take over load to island operation
    bool m_ups_supplyVoltageOK; // True if input voltage is ok
    bool m_ups_runningInIsland; // True if power is provided by internal energy storage
    bool m_ups_energyStorageHighLevel;  // True if more than 85 % energy level is available in storage

    void connectToUPS();
    void disconnectFromUPS();
    QByteArray readFromUPS();
    void tryToParseUPSresponse(QByteArray* buffer);

signals:
    void signal_mainswitchOff();
    void signal_shutdownDueToPowerloss();
    void signal_powerGoodAgain();

    void signal_warning_DCinputVoltageLow();
    void signal_info_DCinputVoltageOK();
    void signal_warning_UPSinIsland();
    void signal_info_UPSonline();
    void signal_alarm_EnergyStorageLow();
    void signal_info_EnergyStorageReady();
    void signal_prewarning_EnergyStorageNotFull();
    void signal_info_EnergyStorageFull();

public slots:
    void slot_startPSUshutdownTimer();

private slots:
    void slot_timer_fired();
};

#endif // UNINTERRUPTIBLEPOWERSUPPLY_H
