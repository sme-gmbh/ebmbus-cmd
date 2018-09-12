#include "loghandler.h"

Loghandler::Loghandler(QObject *parent) : QObject(parent)
{

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
        return new LogEntry(loggingCategory, module, text);
}

void Loghandler::slot_newEntry(LogEntry::LoggingCategory loggingCategory, QString module, QString text)
{
    LogEntry* entry = findOrMakeLogEntry(loggingCategory, module, text);
    m_logentries.append(entry);
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
