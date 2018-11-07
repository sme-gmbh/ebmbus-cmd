/**********************************************************************
** ebmbus-cmd - a commandline tool to control ebm papst fans
** Copyright (C) 2018 Smart Micro Engineering GmbH
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

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
