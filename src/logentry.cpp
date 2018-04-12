#include "logentry.h"

LogEntry::LogEntry(LoggingCategory loggingCategory, QString module, QString text)
{
    m_activeError = false;
    if (loggingCategory == LogEntry::Error)
        m_activeError = true;

    m_loggingCategory = loggingCategory;
    m_module = module;
    m_text = text;
    m_dateTime_triggered = QDateTime::currentDateTime();
}

void LogEntry::quit()
{
    if (!isQuit())
        m_dateTime_quit = QDateTime::currentDateTime();
}

bool LogEntry::isQuit()
{
    if (m_dateTime_quit.isValid())
        return true;
    else
        return false;
}

void LogEntry::setActive()
{
    m_activeError = true;
}

void LogEntry::setInactive()
{
    m_activeError = false;
}

bool LogEntry::isActiveError()
{
    return m_activeError;
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

QDateTime LogEntry::dateTime_triggered() const
{
    return m_dateTime_triggered;
}

QDateTime LogEntry::dateTime_quit() const
{
    return m_dateTime_quit;
}
