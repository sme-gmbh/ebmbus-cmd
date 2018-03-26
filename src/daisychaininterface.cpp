#include "daisychaininterface.h"

DaisyChainInterface::DaisyChainInterface(QObject *parent, RevPiDIO *io, int address_out, int address_in) : QObject(parent)
{
    m_io = io;
    m_address_in = address_in;
    m_address_out = address_out;

    m_old_inputState = false;

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slot_timer_fired()));
    m_timer.start(100);
}

void DaisyChainInterface::slot_setDCIoutput(bool on)
{
    m_io->setBit(m_address_out, on);
}

void DaisyChainInterface::slot_timer_fired()
{
    bool inputState = m_io->getBit(m_address_in);
    if (inputState != m_old_inputState)
    {
        emit signal_DCIloopResponse(inputState);
        m_old_inputState = inputState;
    }
}
