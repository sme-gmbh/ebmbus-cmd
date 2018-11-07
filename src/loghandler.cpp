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

#include "loghandler.h"

Loghandler::Loghandler(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<LogEntry::LoggingCategory>("LogEntry::LoggingCategory");
}

bool Loghandler::hasActiveErrors()
{
    foreach (LogEntry* e, m_logentries)
    {
        if (e->isActiveErrorOrWarning())
            return true;
    }

    return false;
}

QString Loghandler::toString(LogEntry::LoggingCategory category, bool onlyActive)
{
    QString str;
    foreach (LogEntry* e, m_logentries)
    {
        if (e->loggingCategory() != category)
            continue;
        if (!onlyActive || (onlyActive && e->isActive()))
            str += e->toString() + "\n";
    }
    return str;
}

LogEntry *Loghandler::findOrMakeLogEntry(LogEntry::LoggingCategory loggingCategory, QString module, QString text, bool justFind)
{
    foreach(LogEntry* entry, m_logentries)
    {
        if (entry->loggingCategory() != loggingCategory)
            continue;
        if (entry->module() != module)
            continue;
        if (entry->text() != text)
            continue;

        return entry;
    }
    if (justFind)
        return NULL;
    else
    {
        LogEntry* entry = new LogEntry(loggingCategory, module, text);
        m_logentries.append(entry);
        return entry;
    }
}

void Loghandler::slot_newEntry(LogEntry::LoggingCategory loggingCategory, QString module, QString text)
{
    LogEntry* entry = findOrMakeLogEntry(loggingCategory, module, text);
    entry->setActive();
    emit signal_newError();
}

void Loghandler::slot_entryGone(LogEntry::LoggingCategory loggingCategory, QString module, QString text)
{
    // First search the original ticket
    LogEntry* entry = findOrMakeLogEntry(loggingCategory, module, text, true);
    if (entry != NULL)
        entry->setInactive();

    if (!hasActiveErrors())
        emit signal_allErrorsGone();
}

void Loghandler::slot_quitErrors()
{
    foreach (LogEntry* e, m_logentries)
    {
        e->quit();
    }

    if (this->hasActiveErrors())
        emit signal_allErrorsQuit();
    else
        emit signal_allErrorsGone();
}
