#include <stdio.h>
#include "maincontroller.h"


MainController::MainController(QObject *parent) :
    QObject(parent)
{
    printf("ebmBus main controller startup...\n");

    m_ebmbus = new EbmBus(this, "/dev/ttyUSB0");
    m_ebmbus->open();

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slot_timer_fired()));
    m_timer.start(100);
}

void MainController::slot_timer_fired()
{
    static bool on;
    static int speed;

    m_io.setBit(4, m_io.getBit(4)); // Set lamp P1 to status of switch S1
    m_io.setBit(5, on); // Toggle lamp P2
    on = !on;

    if (m_io.getBit(7))
    {
        if (speed < 255)
        {
            speed++;
            m_ebmbus->setSpeedSetpoint(1, 1, speed);
        }
    }

    if (m_io.getBit(8))
    {
        if (speed > 0)
        {
            speed--;
            m_ebmbus->setSpeedSetpoint(1, 1, speed);
        }
    }
}
