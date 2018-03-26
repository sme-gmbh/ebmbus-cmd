#include "uninterruptiblepowersupply.h"

UninterruptiblePowerSupply::UninterruptiblePowerSupply(QObject *parent, RevPiDIO *io, int address_mainswitch) : QObject(parent)
{
    m_io = io;
    m_address_mainswitch = address_mainswitch;

    m_shutdownTimeout = 25000;
    m_mainswitchDelay = 500;

    m_old_mainswitchState = false;
    m_mainswitchOffSignaled = false;

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slot_timer_fired()));
    m_timer.start(100);
}

void UninterruptiblePowerSupply::slot_timer_fired()
{
    bool mainswitchState = m_io->getBit(m_address_mainswitch);
    if (mainswitchState != m_old_mainswitchState)
    {
        if (mainswitchState == false)
        {
            m_dateTime_mainSwitchOff = QDateTime::currentDateTime();
        }
        else
        {
            m_mainswitchOffSignaled = false;
        }
        m_old_mainswitchState = mainswitchState;
    }

    if ((mainswitchState == false) &&
            (m_mainswitchOffSignaled == false) &&
            (m_dateTime_mainSwitchOff.msecsTo(QDateTime::currentDateTime()) > m_mainswitchDelay))
    {
        emit signal_mainswitchOff();
        m_mainswitchOffSignaled = true;
    }
}
