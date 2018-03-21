#include "uninterruptiblepowersupply.h"

UninterruptiblePowerSupply::UninterruptiblePowerSupply(QObject *parent, RevPiDIO *io, int address_mainswitch) : QObject(parent)
{
    m_io = io;
    m_address_mainswitch = address_mainswitch;

    m_shutdownTimeout = 25000;
    m_mainswitchDelay = 500;

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slot_timer_fired()));
    m_timer.start(100);
}

void UninterruptiblePowerSupply::slot_timer_fired()
{
    static bool old_mainswitchState = false;
    static bool mainswitchOffSignaled = false;

    bool mainswitchState = m_io->getBit(m_address_mainswitch);
    if (mainswitchState != old_mainswitchState)
    {
        if (mainswitchState == false)
        {
            m_dateTime_mainSwitchOff = QDateTime::currentDateTime();
        }
        else
        {
            mainswitchOffSignaled = false;
        }
        old_mainswitchState = mainswitchState;
    }

    if ((mainswitchState == false) &&
            (mainswitchOffSignaled == false) &&
            (m_dateTime_mainSwitchOff.msecsTo(QDateTime::currentDateTime()) > m_mainswitchDelay))
    {
        emit signal_mainswitchOff();
        mainswitchOffSignaled = true;
    }
}
