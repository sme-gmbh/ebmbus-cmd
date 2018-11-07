/**********************************************************************
** ebmbus-cmd - a commandline tool to control ebm papst fans
** Copyright (C) 2018 Smart Micro Engineering GmbH
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

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
