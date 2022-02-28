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

#include "ocumodbus.h"
#include <QThread>

OcuModbus::OcuModbus(QObject *parent, QString interface) : QObject(parent)
{
    m_interface = interface;
}

OcuModbus::~OcuModbus()
{
    close();
}

bool OcuModbus::open()
{
    QByteArray interface_ba = m_interface.toLocal8Bit();
    m_bus = modbus_new_rtu(interface_ba.data(), 115200, 'N', 8, 2);
    if (m_bus == nullptr) {
        emit signal_newEntry(LogEntry::Error, "OcuModbus", "Unable to open interface");
        return false;
    }

    if (modbus_connect(m_bus) == -1) {
        emit signal_newEntry(LogEntry::Info, "OcuModbus", QString("Unable to connect to device: ") + QString(modbus_strerror(errno)));
        modbus_free(m_bus);
        return false;
    }

    fprintf(stderr, "OcuModbus::open(): Modbus interface configured and connected.\n");
    return true;
}

void OcuModbus::close()
{
    modbus_close(m_bus);
    modbus_free(m_bus);
}

void OcuModbus::setSlaveAddress(quint16 adr)
{
    modbus_set_slave(m_bus, adr);
}

void OcuModbus::slot_writeHoldingRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg, quint16 rawdata)
{
    int result;
    modbus_set_slave(m_bus, adr);
    // Bus clearance time
    QThread::msleep(100);
    result = modbus_write_register(m_bus, reg, rawdata);
    if (result >= 0)
        emit signal_wroteHoldingRegisterData(telegramID);
    else
    {
        emit signal_newEntry(LogEntry::Info, "OcuModbus", QString("modbus_write_register returned: ") + QString(modbus_strerror(errno) +
                                             QString().sprintf(". adr=%i, reg=%i.", adr, reg)));
        emit signal_transactionLost(telegramID);
    }
}

void OcuModbus::slot_readHoldingRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusHoldingRegister reg)
{
    int result;
    uint16_t rawdata;
    modbus_set_slave(m_bus, adr);
    // Bus clearance time
    QThread::msleep(100);
    result = modbus_read_registers(m_bus, reg, 1, &rawdata);
    if (result >= 0)
        emit signal_receivedHoldingRegisterData(telegramID, adr, reg, rawdata);
    else
    {
        emit signal_newEntry(LogEntry::Info, "OcuModbus", QString("modbus_read_registers returned: ") + QString(modbus_strerror(errno) +
                                            QString().sprintf(". adr=%i, reg=%i.", adr, reg)));
        emit signal_transactionLost(telegramID);
    }
}

void OcuModbus::slot_readInputRegisterData(quint64 telegramID, quint16 adr, OcuModbus::OcuModbusInputRegister reg)
{
    int result;
    uint16_t rawdata;
    modbus_set_slave(m_bus, adr);
    // Bus clearance time
    QThread::msleep(100);
    result = modbus_read_input_registers(m_bus, reg, 1, &rawdata);
    if (result >= 0)
        emit signal_receivedInputRegisterData(telegramID, adr, reg, rawdata);
    else
    {
        emit signal_newEntry(LogEntry::Info, "OcuModbus", QString("modbus_read_input_registers returned: ") + QString(modbus_strerror(errno) +
                                            QString().sprintf(". adr=%i, reg=%i.", adr, reg)));
        emit signal_transactionLost(telegramID);
    }
}
