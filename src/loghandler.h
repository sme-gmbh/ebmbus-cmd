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
