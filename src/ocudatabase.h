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

#ifndef OCUDATABASE_H
#define OCUDATABASE_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QMap>
#include <QDir>
#include <QDirIterator>
#include <QTimer>
#include <QMap>
#include "ocumodbussystem.h"
#include "loghandler.h"
#include "ocufan.h"

// OCUs are managed via Modbus

class OCUdatabase : public QObject
{
    Q_OBJECT
public:
    explicit OCUdatabase(QObject *parent, OcuModbusSystem *ocuModbusSystem, Loghandler *loghandler);

    void loadFromHdd();
    void saveToHdd();
    QString addOCU(int id, int busID, int unit = -1);
    QString deleteOCU(int id);

    QList<OCU*> getOCUs(int busNr = -1);    // If busNr is specified only OCUs of that bus are returned
    OCU* getOCUByID(int id);

    QString getOCUData(int id, QString key);
    QMap<QString,QString> getOCUData(int id, QStringList keys);
    QString setOCUData(int id, QString key, QString value);
    QString setOCUData(int id, QMap<QString,QString> dataMap);

    // Broadcast is not implemented yet
    //QString broadcast(int busID, QMap<QString,QString> dataMap);



private:
    OcuModbusSystem* m_ocuModbusSystem;
    QList<OcuModbus*>* m_ocuModbusList;
    Loghandler* m_loghandler;
    QList<OCU*> m_ocus;
    QTimer m_timer_pollStatus;

    OCU* getOCUByTelegramID(quint64 telegramID);

signals:
    void signal_OCUActualDataHasChanged(int id);

public slots:
    void slot_remoteControlActivated();
    void slot_remoteControlDeactivated();

private slots:
    // High level bus response slots
    void slot_transactionFinished();
    void slot_transactionLost(quint64 telegramID);
    void slot_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg, quint16 rawdata);
    void slot_receivedInputRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusInputRegister reg, quint16 rawdata);
    void slot_wroteHoldingRegisterData(quint64 telegramID);

    // Timer slots
    void slot_timer_pollStatus_fired();
};

#endif // OCUDATABASE_H
