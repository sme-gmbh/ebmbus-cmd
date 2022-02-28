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

    typedef enum {
        HOLDING_REG_D000_Reset = 0xD000,
        HOLDING_REG_D001_DefaultSetValue = 0xD001,
        HOLDING_REG_D002_Password_2 = 0xD002,
        HOLDING_REG_D003_Password_1 = 0xD003,
        HOLDING_REG_D004_Password_0 = 0xD004,
        HOLDING_REG_D005_ControlDefaultSetting = 0xD005,
        HOLDING_REG_D006_ControlCustomerSetting = 0xD006,
        HOLDING_REG_D007_NoFunction = 0xD007,
        HOLDING_REG_D008_Reserved = 0xD008,
        HOLDING_REG_D009_OperatingHoursCounter = 0xD009,
        HOLDING_REG_D00A_OperatingMinutesCounter = 0xD00A,
        HOLDING_REG_D00B_Reserved = 0xD00B
    } OcuModbusHoldingRegister;

    typedef enum {
        INPUT_REG_D000_Identification = 0xD000,
        INPUT_REG_D001_MaximumNumberOfBytes = 0xD001,
        INPUT_REG_D002_SoftwareNameOfBusController = 0xD002,
        INPUT_REG_D003_SoftwareVersionOfBusController = 0xD003,
        INPUT_REG_D004_SoftwareNameOfCommutationController = 0xD004,
        INPUT_REG_D005_SoftwareVersionOfCommutationController = 0xD005
    } OcuModbusInputRegister;

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
