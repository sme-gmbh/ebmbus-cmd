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

#ifndef EBMBUSSYSTEM_H
#define EBMBUSSYSTEM_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QSettings>
#include <libebmbus/ebmbus.h>
#include "revpidio.h"
#include "daisychaininterface.h"

class EbmBusSystem : public QObject
{
    Q_OBJECT
public:
    explicit EbmBusSystem(QObject *parent, RevPiDIO* io);

    QList<EbmBus*> *ebmbuslist();

    EbmBus* getBusByID(int busID);

    QString broadcast(int busID, QMap<QString,QString> dataMap);
    void broadcastSpeed(quint8 speed, bool disableAutosaveAndAutostart = false);

private:
    QList<EbmBus*> m_ebmbuslist;
    QList<DaisyChainInterface*> m_dcilist;
    RevPiDIO* m_io;

signals:

public slots:

private slots:
    void slot_showResponseRaw(quint64 telegramID, quint8 preamble, quint8 commandAndFanaddress, quint8 fanGroup, QByteArray data);
    void slot_transactionLost(quint64 telegramID);
    void slot_transactionFinished();
};

#endif // EBMBUSSYSTEM_H
