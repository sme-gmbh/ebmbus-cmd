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

#include "logentry.h"

LogEntry::LogEntry(LoggingCategory loggingCategory, QString module, QString text)
{
    m_loggingCategory = loggingCategory;
    m_module = module;
    m_text = text;
    m_count = 0;
}

void LogEntry::quit()
{
    if (!m_active)
        return;     // You can not quit non-active errors

    if (!m_dateTime_firstQuit.isValid())
        m_dateTime_firstQuit = QDateTime::currentDateTime();
    else
        m_dateTime_lastQuit = QDateTime::currentDateTime();
}

bool LogEntry::isQuit()
{
    if (m_dateTime_firstQuit.isValid() || m_dateTime_lastQuit.isValid())
        return true;
    else
        return false;
}

void LogEntry::setActive()
{
    m_active = true;
    m_count++;
    if (!m_dateTime_firstTriggered.isValid())
        m_dateTime_firstTriggered = QDateTime::currentDateTime();
    m_dateTime_lastTriggered = QDateTime::currentDateTime();
}

void LogEntry::setInactive()
{
    m_active = false;
    if(!m_dateTime_firstGone.isValid())
        m_dateTime_firstGone = QDateTime::currentDateTime();
    m_dateTime_lastGone = QDateTime::currentDateTime();
}

bool LogEntry::isActiveErrorOrWarning()
{
    if ((m_loggingCategory == LogEntry::Error) || (m_loggingCategory == LogEntry::Warning))
        return m_active;
    else
        return false;
}

bool LogEntry::isActive() const
{
    return m_active;
}

quint64 LogEntry::getCount()
{
    return m_count;
}

LogEntry::LoggingCategory LogEntry::loggingCategory() const
{
    return m_loggingCategory;
}

QString LogEntry::module() const
{
    return m_module;
}

QString LogEntry::text() const
{
    return m_text;
}

QDateTime LogEntry::dateTime_lastTriggered() const
{
    return m_dateTime_lastTriggered;
}

QDateTime LogEntry::dateTime_lastQuit() const
{
    return m_dateTime_lastQuit;
}

QString LogEntry::toString()
{
    QString str;

    switch (m_loggingCategory)
    {
    case LogEntry::Info:
        str = "Info from ";
        break;
    case LogEntry::Warning:
        str = "Warning from ";
        break;
    case LogEntry::Error:
        str = "Error from ";
        break;
    }

    str += m_module + ": " + m_text;
    str += " Occurred first " + m_dateTime_firstTriggered.toString() + ".";
    if (m_dateTime_firstQuit.isValid())
    {
        str += " It was first quit " + m_dateTime_firstQuit.toString();
        if (m_dateTime_lastQuit.isValid())
            str += " and last quit " + m_dateTime_lastQuit.toString();
        str += ".";
    }

    if (m_dateTime_firstGone.isValid())
        str += " It was first gone " + m_dateTime_firstGone.toString() + ".";

    if (m_count > 1)
    {
        str += " In sum it occurred " + QString().setNum(m_count) + " times, last " + m_dateTime_lastTriggered.toString();
        if (m_dateTime_lastGone.isValid() && !m_active)
            str += " but is gone since " + m_dateTime_lastGone.toString() + ".";
        else
            str += " and is still active.";
    }

    return str;
}
