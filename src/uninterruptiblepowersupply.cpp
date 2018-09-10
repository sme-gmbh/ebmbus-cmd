#include "uninterruptiblepowersupply.h"
#include <stdio.h>
#include <unistd.h>

UninterruptiblePowerSupply::UninterruptiblePowerSupply(QObject *parent, RevPiDIO *io, int address_mainswitch) : QObject(parent)
{
    m_io = io;
    m_address_mainswitch = address_mainswitch;

    m_ftdi = NULL;

    setShutdownTimeout(30000);
    setPowerGoodDelay(5000);
    m_mainswitchDelay = 500;

    m_old_mainswitchState = false;
    m_mainswitchOffSignaled = false;

    m_ups_communicationOK = true;

    m_ups_bufferReady = false;
    m_ups_energyStorageHighLevel = false;
    m_ups_runningInIsland = false;
    m_ups_supplyVoltageOK = true;

    connectToUPS();

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slot_timer_fired()));
    m_timer.start(100);

    connect(this, SIGNAL(signal_ups_communicationHeartbeat()), &m_ups_communicationWatchdogTimer, SLOT(start()));
    connect(&m_ups_communicationWatchdogTimer, SIGNAL(timeout()), this, SIGNAL(signal_ups_communicationFailure()));
    connect(&m_ups_communicationWatchdogTimer, SIGNAL(timeout()), this, SLOT(slot_ups_communicationFailure()));
    m_ups_communicationWatchdogTimer.setInterval(5000);

    m_shutdownTimer.setSingleShot(true);

    // Start the shutdown timer as soon as we lost significant level of energy in storage
    connect(this, SIGNAL(signal_prewarning_EnergyStorageNotFull()), &m_shutdownTimer, SLOT(start()));
    // Abort shutdown if necessary energy level could be restored due to restored mains power
    connect(this, SIGNAL(signal_info_EnergyStorageFull()), &m_shutdownTimer, SLOT(stop()));
    // Finally if timer fires, initiate system shutdown - now way back now - system going down
    connect(&m_shutdownTimer, SIGNAL(timeout()), this, SLOT(slot_startPSUshutdownTimer()));
    connect(&m_shutdownTimer, SIGNAL(timeout()), this, SIGNAL(signal_shutdownDueToPowerloss()));

    m_powerGoodTimer.setSingleShot(true);
    // If power has returned, start wait timer to watch if power is ok
    connect(this, SIGNAL(signal_info_DCinputVoltageOK()), &m_powerGoodTimer, SLOT(start()));
    // If power was ok for that amount of time, signal powerGoodAgain
    connect(&m_powerGoodTimer, SIGNAL(timeout()), this, SIGNAL(signal_powerGoodAgain()));
    // And stop shutdown timer, if active
    connect(&m_powerGoodTimer, SIGNAL(timeout()), &m_shutdownTimer, SLOT(stop()));
    // If power fails again, stop powerGood timer in case
    connect(this, SIGNAL(signal_warning_DCinputVoltageLow()), &m_powerGoodTimer, SLOT(stop()));
}

void UninterruptiblePowerSupply::setShutdownTimeout(int milliseconds)
{
    m_shutdownTimer.setInterval(milliseconds);
}

void UninterruptiblePowerSupply::setMainswitchDelay(int milliseconds)
{
    m_mainswitchDelay = milliseconds;
}

void UninterruptiblePowerSupply::setPowerGoodDelay(int milliseconds)
{
    m_powerGoodTimer.setInterval(milliseconds);
}

// This tells the SITOP UPS500S to start its shutdown timer
// Notice that USB polling must stop (== System must actually shut down) to start the shutdown of the ups after calling this method
void UninterruptiblePowerSupply::slot_startPSUshutdownTimer()
{
    m_timer.stop();
    sleep(1);

    unsigned char c[1];
    c[0] = 'R';
    ftdi_write_data(m_ftdi, c, 1);

    sleep(2);

    disconnectFromUPS();
}

void UninterruptiblePowerSupply::connectToUPS()
{
    int baudrate = 9600;

    int ret;

    if ((m_ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        m_ftdi = NULL;
        return;
    }

    if ((ret = ftdi_usb_open(m_ftdi, 0x0403, 0xe0e4)) < 0)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(m_ftdi));
        ftdi_free(m_ftdi);
        m_ftdi = NULL;
        return;
    }

    // Set baudrate
    ret = ftdi_set_baudrate(m_ftdi, baudrate);
    if (ret < 0)
    {
        fprintf(stderr, "unable to set baudrate: %d (%s)\n", ret, ftdi_get_error_string(m_ftdi));
        return;
    }

    // Set line parameters
    ret = ftdi_set_line_property(m_ftdi, BITS_8, STOP_BIT_1, NONE);
    if (ret < 0)
    {
        fprintf(stderr, "unable to set line parameters: %d (%s)\n", ret, ftdi_get_error_string(m_ftdi));
        return;
    }

    if (m_ups_communicationOK == false)
    {
        fprintf(stderr, "Successfully reconnected usb to UPS...\n");
    }

    ftdi_setdtr(m_ftdi, 1);
    ftdi_setrts(m_ftdi, 1);
    m_ups_communicationWatchdogTimer.start();
}

void UninterruptiblePowerSupply::disconnectFromUPS()
{
    int ret;

    if (m_ftdi == NULL)
        return;

    ftdi_setdtr(m_ftdi, 0);
    ftdi_setrts(m_ftdi, 0);

    if ((ret = ftdi_usb_close(m_ftdi)) < 0)
    {
        fprintf(stderr, "unable to close ftdi device: %d (%s)\n", ret, ftdi_get_error_string(m_ftdi));
        ftdi_free(m_ftdi);
        return;
    }

    ftdi_free(m_ftdi);
    m_ups_communicationWatchdogTimer.stop();
}

QByteArray UninterruptiblePowerSupply::readFromUPS()
{
    int ret;
    unsigned char buf[1024];
    QByteArray response;

    if (m_ftdi == NULL)
        return QByteArray();

    do
    {
        ret = ftdi_read_data(m_ftdi, buf, sizeof(buf));
        if (ret > 0)
            response += QByteArray((char*)buf, ret);
    }while(ret > 0);

    return response;
}

void UninterruptiblePowerSupply::tryToParseUPSresponse(QByteArray *buffer)
{
    int posOfNewline;
    do
    {
        posOfNewline = buffer->indexOf('\n');
        if (posOfNewline < 0)
            return;

        QByteArray line = buffer->left(posOfNewline);
        buffer->remove(0, posOfNewline + 1);

        if (line.startsWith("BUFRD"))
        {
            if (!m_ups_bufferReady)
            {
                m_ups_bufferReady = true;
                emit signal_info_EnergyStorageReady();
            }
            emit signal_ups_communicationHeartbeat();
        }
        else if (line.startsWith("ALARM"))
        {
            if (m_ups_bufferReady)
            {
                m_ups_bufferReady = false;
                emit signal_alarm_EnergyStorageLow();
            }
            emit signal_ups_communicationHeartbeat();
        }
        else if (line.startsWith("DC_OK"))
        {
            if (!m_ups_supplyVoltageOK)
            {
                m_ups_supplyVoltageOK = true;
                emit signal_info_DCinputVoltageOK();
            }
            emit signal_ups_communicationHeartbeat();
        }
        else if (line.startsWith("DC_LO"))
        {
            if (m_ups_supplyVoltageOK)
            {
                m_ups_supplyVoltageOK = false;
                emit signal_warning_DCinputVoltageLow();
            }
            emit signal_ups_communicationHeartbeat();
        }
        else if (line.startsWith("*****"))
        {
            if (m_ups_runningInIsland)
            {
                m_ups_runningInIsland = false;
                emit signal_info_UPSonline();
            }
            emit signal_ups_communicationHeartbeat();
        }
        else if (line.startsWith("*BAT*"))
        {
            if (!m_ups_runningInIsland)
            {
                m_ups_runningInIsland = true;
                emit signal_warning_UPSinIsland();
            }
            emit signal_ups_communicationHeartbeat();
        }
        else if (line.startsWith("BA>85"))
        {
            if (!m_ups_energyStorageHighLevel)
            {
                m_ups_energyStorageHighLevel = true;
                emit signal_info_EnergyStorageFull();
            }
            emit signal_ups_communicationHeartbeat();
        }
        else if (line.startsWith("BA<85"))
        {
            if (m_ups_energyStorageHighLevel)
            {
                m_ups_energyStorageHighLevel = false;
                emit signal_prewarning_EnergyStorageNotFull();
            }
            emit signal_ups_communicationHeartbeat();
        }
    }while(posOfNewline >= 0);

    // Just to make sure nothing goes wrong in case...
    if (buffer->length() > 10000)
        buffer->clear();
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
        slot_startPSUshutdownTimer();
        emit signal_mainswitchOff();
        m_mainswitchOffSignaled = true;
    }

    m_upsResponseBuffer += readFromUPS();
    tryToParseUPSresponse(&m_upsResponseBuffer);
}

void UninterruptiblePowerSupply::slot_ups_communicationHeartbeat()
{
    if (m_ups_communicationOK == false)
    {
        fprintf(stderr, "Successfully reestablished communication to UPS.\n");
    }
    m_ups_communicationOK = true;
}

void UninterruptiblePowerSupply::slot_ups_communicationFailure()
{
    m_ups_communicationOK = false;
    fprintf(stderr, "UPS communication failure - trying to reconnect...\n");
    disconnectFromUPS();
    connectToUPS();
}
