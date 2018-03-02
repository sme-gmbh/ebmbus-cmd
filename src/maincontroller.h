#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QTimer>
#include "revpidio.h"

class MainController : public QObject
{
    Q_OBJECT

public:
    explicit MainController(QObject *parent = 0);

private:
    RevPiDIO m_io;
    QTimer m_timer;

private slots:
    void slot_timer_fired();
};

#endif // MAINCONTROLLER_H
