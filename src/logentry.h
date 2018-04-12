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
    bool isActiveError();

    LoggingCategory loggingCategory() const;
    QString module() const;
    QString text() const;
    QDateTime dateTime_triggered() const;
    QDateTime dateTime_quit() const;

private:
    LoggingCategory m_loggingCategory;
    QString m_module;
    QString m_text;
    bool m_activeError;
    QDateTime m_dateTime_triggered; // This holds the time when the event was triggered
    QDateTime m_dateTime_quit;      // This holds the time when the event was quit by an operator.
};

#endif // LOGENTRY_H
