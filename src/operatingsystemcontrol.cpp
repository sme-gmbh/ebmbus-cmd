#include "operatingsystemcontrol.h"

OperatingSystemControl::OperatingSystemControl(QObject *parent) : QObject(parent)
{

}

void OperatingSystemControl::slot_shutdownNOW()
{
    QProcess p;

    p.startDetached("sudo poweroff");
}
