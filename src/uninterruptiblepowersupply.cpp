#include "uninterruptiblepowersupply.h"
#include <stdio.h>
#include <unistd.h>
#include <libftdi1/ftdi.h>

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

// This tells the SITOP UPS500S to start its shutdown timer
void UninterruptiblePowerSupply::startPSUshutdownTimer()
{
    unsigned char buf[1024];
    int baudrate = 9600;

    int ret;
    struct ftdi_context *ftdi;
    struct ftdi_version_info version;
    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        return EXIT_FAILURE;
    }

    version = ftdi_get_library_version();
    printf("Initialized libftdi %s (major: %d, minor: %d, micro: %d, snapshot ver: %s)\n",
           version.version_str, version.major, version.minor, version.micro,
           version.snapshot_str);

    if ((ret = ftdi_usb_open(ftdi, 0x0403, 0xe0e4)) < 0)
    {
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        return EXIT_FAILURE;
    }

    // Read out FTDIChip-ID of R type chips
    if (ftdi->type == TYPE_BM)
    {
        unsigned int chipid;
        printf("ftdi_read_chipid: %d\n", ftdi_read_chipid(ftdi, &chipid));
        printf("FTDI chipid: %X\n", chipid);
    }

    // Set baudrate
    ret = ftdi_set_baudrate(ftdi, baudrate);
    if (ret < 0)
    {
        fprintf(stderr, "unable to set baudrate: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        exit(-1);
    }

    // Set line parameters
    ret = ftdi_set_line_property(ftdi, BITS_8, STOP_BIT_1, NONE);
    if (ret < 0)
    {
        fprintf(stderr, "unable to set line parameters: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        exit(-1);
    }

    ftdi_setdtr(ftdi, 1);
    ftdi_setrts(ftdi, 1);


//    int i=500;
//    while (i!=0)
//    {
//        i--;
//        ret = ftdi_read_data(ftdi, buf, sizeof(buf));
//        if (ret<0)
//            sleep(1);
//        else if(ret> 0)
//        {
//            fwrite(buf, ret, 1, stdout);
//            fflush(stdout);
//        }
//    }

    unsigned char c[1];
    c[0] = 'R';
    ret = ftdi_write_data(ftdi, c, 1);

//    i=500;
//    while (i!=0)
//    {
//        i--;
//        ret = ftdi_read_data(ftdi, buf, sizeof(buf));
//        if (ret<0)
//            sleep(1);
//        else if(ret> 0)
//        {
//            fwrite(buf, ret, 1, stdout);
//            fflush(stdout);
//        }
//    }

    sleep(2);

    c[0] = 'R';
//    ret = ftdi_write_data(ftdi, c, 1);



    ftdi_setdtr(ftdi, 0);
    ftdi_setrts(ftdi, 0);

    if ((ret = ftdi_usb_close(ftdi)) < 0)
    {
        fprintf(stderr, "unable to close ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        return EXIT_FAILURE;
    }

    ftdi_free(ftdi);
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
        startPSUshutdownTimer();
        emit signal_mainswitchOff();
        m_mainswitchOffSignaled = true;
    }
}
