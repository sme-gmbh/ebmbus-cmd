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

#include "ebmmodbus.h"
#include <QThread>

EbmModbus::EbmModbus(QObject *parent, QString interface) : QObject(parent)
{
    m_interface = interface;
}

EbmModbus::~EbmModbus()
{
    close();
}

bool EbmModbus::open()
{
    QByteArray interface_ba = m_interface.toLocal8Bit();
    m_bus = modbus_new_rtu(interface_ba.data(), 19200, 'E', 8, 1);
    if (m_bus == NULL) {
        emit signal_newEntry(LogEntry::Error, "EbmModbus", "Unable to open interface");
        return false;
    }

    if (modbus_connect(m_bus) == -1) {
        emit signal_newEntry(LogEntry::Info, "EbmModbus", QString("Unable to connect to device: ") + QString(modbus_strerror(errno)));
        modbus_free(m_bus);
        return false;
    }

    fprintf(stderr, "EbmModbus::open(): Modbus interface configured and connected.\n");
    return true;
}

void EbmModbus::close()
{
    modbus_close(m_bus);
    modbus_free(m_bus);
}

void EbmModbus::setSlaveAddress(quint16 adr)
{
    modbus_set_slave(m_bus, adr);
}

void EbmModbus::slot_writeHoldingRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg, quint16 rawdata)
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
        emit signal_newEntry(LogEntry::Info, "EbmModbus", QString("modbus_write_register returned: ") + QString(modbus_strerror(errno) +
                                             QString().sprintf(". adr=%i, reg=%i.", adr, reg)));
        emit signal_transactionLost(telegramID);
    }
}

void EbmModbus::slot_readHoldingRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg)
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
        emit signal_newEntry(LogEntry::Info, "EbmModbus", QString("modbus_read_registers returned: ") + QString(modbus_strerror(errno) +
                                            QString().sprintf(". adr=%i, reg=%i.", adr, reg)));
        emit signal_transactionLost(telegramID);
    }
}

void EbmModbus::slot_readInputRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusInputRegister reg)
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
        emit signal_newEntry(LogEntry::Info, "EbmModbus", QString("modbus_read_input_registers returned: ") + QString(modbus_strerror(errno) +
                                            QString().sprintf(". adr=%i, reg=%i.", adr, reg)));
        emit signal_transactionLost(telegramID);
    }
}
