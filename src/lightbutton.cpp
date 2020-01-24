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

#include "lightbutton.h"

LightButton::LightButton(QObject *parent, RevPiDIO *io, int address_light, int address_button) : QObject(parent)
{
    m_io = io;
    m_address_light = address_light;
    m_address_button = address_button;
    m_LEDstatus = LED_OFF;
    m_old_buttonState = false;
    m_button_pressed = false;
    m_clickTime = 1000; // ClickTime defaults to 1000 ms
    m_blinkFrequency = 1000; // Blink frequency defaults to 1 Hz

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slot_timer_fired()));
    m_timer.start(20);
}

int LightButton::pressed_milliseconds()
{
    if (m_button_pressed)
        return (m_dateTime_lastPress.msecsTo(QDateTime::currentDateTime()));
    else
        return 0;
}

void LightButton::setClickTime(int milliseconds)
{
    m_clickTime = milliseconds;
}

void LightButton::setBlinkFrequency(int milliHertz)
{
    m_blinkFrequency = milliHertz;
}

void LightButton::slot_setLight(LED_Status status)
{
    m_LEDstatus = status;
}

void LightButton::slot_timer_fired()
{
    // Read in the button state and eventually emit signals

    bool buttonState = m_io->getBit(m_address_button);

    if (buttonState != m_old_buttonState)
    {
        if (buttonState == true)
        {
            m_dateTime_lastPress = QDateTime::currentDateTime();
            m_button_pressed = true;
            emit signal_button_pressed();
        }
        else
        {
            int ms = pressed_milliseconds();
            m_button_pressed = false;
            emit signal_button_released();
            if (ms < m_clickTime)
                emit signal_button_clicked();
        }
        m_old_buttonState = buttonState;
    }

    // Write out led state

    switch (m_LEDstatus)
    {
    case LED_OFF:
        m_io->setBit(m_address_light, false);
        break;
    case LED_ON:
        m_io->setBit(m_address_light, true);
        break;
    case LED_BLINK:
        quint64 ms_since_epoch = QDateTime::currentMSecsSinceEpoch();

        if ((ms_since_epoch % m_blinkFrequency) > (m_blinkFrequency / 2UL))
            m_io->setBit(m_address_light, true);
        else
            m_io->setBit(m_address_light, false);
        break;
    }
}
