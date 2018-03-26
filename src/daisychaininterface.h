#ifndef DAISYCHAININTERFACE_H
#define DAISYCHAININTERFACE_H

#include <QObject>
#include <QTimer>
#include "revpidio.h"

class DaisyChainInterface : public QObject
{
    Q_OBJECT
public:
    explicit DaisyChainInterface(QObject *parent, RevPiDIO* io, int address_out, int address_in);

private:
    RevPiDIO* m_io;
    int m_address_in;
    int m_address_out;
    QTimer m_timer;
    bool m_old_inputState;

signals:
    void signal_DCIloopResponse(bool on);

public slots:
    void slot_setDCIoutput(bool on);

private slots:
    void slot_timer_fired();
};

#endif // DAISYCHAININTERFACE_H
