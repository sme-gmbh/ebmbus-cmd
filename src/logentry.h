#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <QString>
#include <QDateTime>

class LogEntry
{
public:
    typedef enum {
        Info,
        Warning,
        Error
    } LoggingCategory;

    LogEntry(LoggingCategory loggingCategory, QString module, QString text);

    void quit();
    bool isQuit();

    void setActive();
    void setInactive();
    bool isActiveErrorOrWarning();
    bool isActive() const;

    quint64 getCount();

    LoggingCategory loggingCategory() const;
    QString module() const;
    QString text() const;
    QDateTime dateTime_lastTriggered() const;
    QDateTime dateTime_lastQuit() const;

    QString toString();

private:
    LoggingCategory m_loggingCategory;
    QString m_module;
    QString m_text;
    bool m_active;
    quint64 m_count;                    // This increments every time the entry is set active.
    QDateTime m_dateTime_firstTriggered;// This holds the first time when the event was triggered.
    QDateTime m_dateTime_firstQuit;     // This holds the first time when the event was quit by an operator.
    QDateTime m_dateTime_firstGone;     // This holds the first time when the error was gone.
    QDateTime m_dateTime_lastTriggered; // This holds the last time when the event was triggered.
    QDateTime m_dateTime_lastQuit;      // This holds the last time when the event was quit by an operator.
    QDateTime m_dateTime_lastGone;      // This holds the last time when the error was gone.
};

#endif // LOGENTRY_H
