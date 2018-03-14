#include <stdio.h>
#include "maincontroller.h"


MainController::MainController(QObject *parent) :
    QObject(parent)
{
    printf("ebmBus main controller startup...\n");

    m_ebmbus = new EbmBus(this, "/dev/ttyUSB0");
    m_ebmbus->open();

    connect(m_ebmbus, SIGNAL(signal_DaisyChainAdressingFinished()), this, SLOT(slot_daisyChainAddressingFinished()));
    connect(m_ebmbus, SIGNAL(signal_setDCIoutput(bool)), this, SLOT(slot_setDCIsignal(bool)));
    connect(m_ebmbus, SIGNAL(signal_response(quint8,quint8,quint8,QByteArray)), this, SLOT(slot_showResponse(quint8,quint8,quint8,QByteArray)));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slot_timer_fired()));
    m_timer.start(100);

    m_io.setBit(4, false);    // Green LED off
    m_io.setBit(5, true);   // Red LED on
}

void MainController::slot_timer_fired()
{
    static int speed;

    if (m_io.getBit(8))
    {
        if (speed < 255)
        {
            speed++;
            m_ebmbus->setSpeedSetpoint(0, 0, speed);
        }
    }

    if (m_io.getBit(7))
    {
        if (speed > 0)
        {
            speed--;
            m_ebmbus->setSpeedSetpoint(0, 0, speed);
        }
    }

    if (m_io.getBit(6))
    {
        if (speed > 0)
        {
            speed = 0;
            m_ebmbus->setSpeedSetpoint(0, 0, speed);
        }
    }


    // Test DCI-Adressing
    if (m_io.getBit(4) && !m_ebmbus->isDaisyChainInProgress())
    {
        printf("Start DCI Addressing...\n");
        m_io.setBit(4, false);    // Green LED off
        m_io.setBit(5, true);   // Red LED on
        m_ebmbus->startDaisyChainAddressing();
    }

    // Test DCI-DeAdressing
    if (m_io.getBit(5) && !m_ebmbus->isDaisyChainInProgress())
    {
        printf("Start DCI Addressing...\n");
        m_io.setBit(4, false);    // Green LED off
        m_io.setBit(5, true);   // Red LED on
        m_ebmbus->clearAllAddresses();
    }

    // Get dci loop back signal
    bool dciLoopBack = m_io.getBit(0);
    static bool dciLoopBack_old = false;

    if (dciLoopBack != dciLoopBack_old)
    {
        dciLoopBack_old = dciLoopBack;
        m_ebmbus->slot_DCIloopResponse(dciLoopBack);
    }
}

void MainController::slot_daisyChainAddressingFinished()
{
    m_io.setBit(5, false);   // Red LED off
    m_io.setBit(4, true);    // Green LED on
    printf("DCI Addressing finished.\n");
}

void MainController::slot_setDCIsignal(bool on)
{
    m_io.setBit(0, on);
}

void MainController::slot_showResponse(quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data)
{
    printf("PRE: %02X  commandAndFanaddress: %02X  fanGroup: %02X  data: ", preamble, commandAndFanaddress, fanGroup);
    foreach (quint8 byte, data)
    {
        printf("%02X ", byte);
    }
    printf("\n");
}
