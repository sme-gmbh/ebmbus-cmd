#include "loghandler.h"

Loghandler::Loghandler(QObject *parent) : QObject(parent)
{

}

bool Loghandler::hasActiveErrors()
{
    foreach (LogEntry* e, m_logentries)
    {
        if (e->isActiveError())
            return true;
    }

    return false;
}

void Loghandler::slot_newEntry(LogEntry::LoggingCategory loggingCategory, QString module, QString text)
{
    LogEntry* newEntry = new LogEntry(loggingCategory, module, text);
    m_logentries.append(newEntry);
    emit signal_newError();
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
