/**********************************************************************
** ebmbus-cmd - a commandline tool to control ebm papst fans
** Copyright (C) 2022 Smart Micro Engineering GmbH
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

#ifndef OCUMODBUS_H
#define OCUMODBUS_H

#include <QObject>
#include <loghandler.h>
#include <modbus/modbus-rtu.h>

class OcuModbus : public QObject
{
    Q_OBJECT
public:
    explicit OcuModbus(QObject *parent, QString interface);
    ~OcuModbus();


    bool open();
    void close();
    void setSlaveAddress(quint16 adr);

signals:

private:
    QString m_interface;
    modbus_t *m_bus;

public slots:
    void slot_writeHoldingRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg, quint16 rawdata);
    void slot_readHoldingRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg);
    void slot_readInputRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusInputRegister reg);

signals:
    // Modbus result signals
    void signal_transactionLost(quint64 telegramID);
    void signal_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg, quint16 rawdata);
    void signal_receivedInputRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusInputRegister reg, quint16 rawdata);
    void signal_wroteHoldingRegisterData(quint64 telegramID);

    // Log output signals
    void signal_newEntry(LogEntry::LoggingCategory loggingCategory, QString module, QString text);
    void signal_entryGone(LogEntry::LoggingCategory loggingCategory, QString module, QString text);
};

#endif // OCUMODBUS_H
