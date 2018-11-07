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

#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QObject>
#include <QList>
#include <QString>

#include "logentry.h"

class Loghandler : public QObject
{
    Q_OBJECT
public:
    explicit Loghandler(QObject *parent = 0);

    bool hasActiveErrors();
    QString toString(LogEntry::LoggingCategory category, bool onlyActive = false);

private:
    QList<LogEntry*> m_logentries;
    LogEntry* findOrMakeLogEntry(LogEntry::LoggingCategory loggingCategory, QString module, QString text, bool justFind = false);

signals:

    void signal_newError();
    void signal_allErrorsQuit();
    void signal_allErrorsGone();

public slots:
    void slot_newEntry(LogEntry::LoggingCategory loggingCategory, QString module, QString text);
    void slot_entryGone(LogEntry::LoggingCategory loggingCategory, QString module, QString text);
    void slot_quitErrors();
};

#endif // LOGHANDLER_H
