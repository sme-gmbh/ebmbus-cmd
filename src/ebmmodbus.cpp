#include "ebmmodbus.h"

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
    result = modbus_write_register(m_bus, reg, rawdata);
    if (result >= 0)
        emit signal_wroteHoldingRegisterData(telegramID);
    else
    {
        emit signal_newEntry(LogEntry::Info, "EbmModbus", QString("modbus_write_register returned: ") + QString(modbus_strerror(errno) +
                                             QString().sprintf(". adr=%i, reg=%i.", telegramID, adr, reg)));
        emit signal_transactionLost(telegramID);
    }
}

void EbmModbus::slot_readHoldingRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg)
{
    int result;
    uint16_t rawdata;
    modbus_set_slave(m_bus, adr);
    result = modbus_read_registers(m_bus, reg, 1, &rawdata);
    if (result >= 0)
        emit signal_receivedHoldingRegisterData(telegramID, adr, reg, rawdata);
    else
    {
        emit signal_newEntry(LogEntry::Info, "EbmModbus", QString("modbus_read_registers returned: ") + QString(modbus_strerror(errno) +
                                            QString().sprintf(". adr=%i, reg=%i.", telegramID, adr, reg)));
        emit signal_transactionLost(telegramID);
    }
}

void EbmModbus::slot_readInputRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusInputRegister reg)
{
    int result;
    uint16_t rawdata;
    modbus_set_slave(m_bus, adr);
    result = modbus_read_input_registers(m_bus, reg, 1, &rawdata);
    if (result >= 0)
        emit signal_receivedInputRegisterData(telegramID, adr, reg, rawdata);
    else
    {
        emit signal_newEntry(LogEntry::Info, "EbmModbus", QString("modbus_read_input_registers returned: ") + QString(modbus_strerror(errno) +
                                            QString().sprintf(". adr=%i, reg=%i.", telegramID, adr, reg)));
        emit signal_transactionLost(telegramID);
    }
}
