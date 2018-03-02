#include <stdio.h>
#include "maincontroller.h"


MainController::MainController(QObject *parent) :
    QObject(parent)
{
    printf("ebmBus main controller startup...\n");

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slot_timer_fired()));
    m_timer.start(500);
}

void MainController::slot_timer_fired()
{
    static bool on;
    m_io.setBit(4, m_io.getBit(4)); // Set lamp P1 to status of switch S1
    m_io.setBit(4, on); // Toggle lamp P2
    on = !on;
}
