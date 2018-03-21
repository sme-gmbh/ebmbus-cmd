#ifndef OPERATINGSYSTEMCONTROL_H
#define OPERATINGSYSTEMCONTROL_H

#include <QObject>
#include <QProcess>

class OperatingSystemControl : public QObject
{
    Q_OBJECT
public:
    explicit OperatingSystemControl(QObject *parent = 0);

signals:

public slots:
    void slot_shutdownNOW();
};

#endif // OPERATINGSYSTEMCONTROL_H
