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

#ifndef EBMMODBUS_H
#define EBMMODBUS_H

#include <QObject>
#include <loghandler.h>
#include <modbus/modbus-rtu.h>

class EbmModbus : public QObject
{
    Q_OBJECT
public:
    explicit EbmModbus(QObject *parent, QString interface);
    ~EbmModbus();


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
        HOLDING_REG_D00B_Reserved = 0xD00B,
        HOLDING_REG_D100_FanAddress = 0xD100,
        HOLDING_REG_D101_SourceSetValue = 0xD101,
        HOLDING_REG_D102_PreferredRunningDirection = 0xD102,
        HOLDING_REG_D103_SaveSetValue = 0xD103,
        HOLDING_REG_D104_NoFunction = 0xD104,
        HOLDING_REG_D105_InternalParameterSet = 0xD105,
        HOLDING_REG_D106_ControlMode_ParameterSet_1 = 0xD106,
        HOLDING_REG_D107_ControlMode_ParameterSet_2 = 0xD107,
        HOLDING_REG_D108_NoFunction = 0xD108,
        HOLDING_REG_D109_NoFunction = 0xD109,
        HOLDING_REG_D10A_Pfactor_ParameterSet1 = 0xD10A,
        HOLDING_REG_D10B_Pfactor_ParameterSet2 = 0xD10B,
        HOLDING_REG_D10C_Ifactor_ParameterSet1 = 0xD10C,
        HOLDING_REG_D10D_Ifactor_ParameterSet2 = 0xD10D,
        HOLDING_REG_D10E_MaximumModulationLevel_ParameterSet1 = 0xD10E,
        HOLDING_REG_D10F_MaximumModulationLevel_ParameterSet2 = 0xD10F,
        HOLDING_REG_D110_MinimumModulationLevel_ParameterSet1 = 0xD110,
        HOLDING_REG_D111_MinimumModulationLevel_ParameterSet2 = 0xD111,
        HOLDING_REG_D112_MotorStopEnable_ParameterSet1 = 0xD112,
        HOLDING_REG_D113_MotorStopEnable_ParameterSet2 = 0xD113,
        HOLDING_REG_D114_SetValue_ParameterSet1 = 0xD114,
        HOLDING_REG_D115_SetValue_ParameterSet2 = 0xD115,
        HOLDING_REG_D116_StartingModulationLevel = 0xD116,
        HOLDING_REG_D117_MaxPermissibleModulationLevel = 0xD117,
        HOLDING_REG_D118_MinPermissibleModulationLevel = 0xD118,
        HOLDING_REG_D119_MaximumSpeed = 0xD119,
        HOLDING_REG_D11A_MaxPermissibleSpeed = 0xD11A,
        HOLDING_REG_D11B_Reserved = 0xD11B,
        HOLDING_REG_D11C_Reserved = 0xD11C,
        HOLDING_REG_D11D_Vacant = 0xD11D,
        HOLDING_REG_D11E_NoFunction = 0xD11E,
        HOLDING_REG_D11F_RampUpTime = 0xD11F,
        HOLDING_REG_D120_RampDownTime = 0xD120,
        HOLDING_REG_D121_Reserved = 0xD121,
        HOLDING_REG_D122_Reserved = 0xD122,
        HOLDING_REG_D123_Reserved = 0xD123,
        HOLDING_REG_D124_Reserved = 0xD124,
        HOLDING_REG_D125_Reserved = 0xD125,
        HOLDING_REG_D126_Reserved = 0xD126,
        HOLDING_REG_D127_Reserved = 0xD127,
        HOLDING_REG_D128_LimitSpeed = 0xD128,
        HOLDING_REG_D129_Reserved = 0xD129,
        HOLDING_REG_D12A_PotentiometerCharacteristicPoint_1X_ParameterSet1 = 0xD12A,
        HOLDING_REG_D12B_PotentiometerCharacteristicPoint_1Y_ParameterSet1 = 0xD12B,
        HOLDING_REG_D12C_PotentiometerCharacteristicPoint_2X_ParameterSet1 = 0xD12C,
        HOLDING_REG_D12D_PotentiometerCharacteristicPoint_2Y_ParameterSet1 = 0xD12D,
        HOLDING_REG_D12E_NoFunction = 0xD12E,
        HOLDING_REG_D12F_ControlLimitation = 0xD12F,
        HOLDING_REG_D130_NoFunction = 0xD130,
        HOLDING_REG_D131_Reserved = 0xD131,
        HOLDING_REG_D132_NoFunction = 0xD132,
        HOLDING_REG_D133_Reserved = 0xD133,
        HOLDING_REG_D134_Reserved = 0xD134,
        HOLDING_REG_D135_MaxPermittedPower = 0xD135,
        HOLDING_REG_D136_MaximumPowerAtDeratingEnd = 0xD136,
        HOLDING_REG_D137_ModuleTemperatureAtStartOfDerating = 0xD137,
        HOLDING_REG_D138_ModuleTemperatureAtEndOfDerating = 0xD138,
        HOLDING_REG_D139_Reserved = 0xD139,
        HOLDING_REG_D13A_Reserved = 0xD13A,
        HOLDING_REG_D13B_MaximumCoilCurrent = 0xD13B,
        HOLDING_REG_D13C_PotentiometerCharacteristicPoint_1X_ParameterSet2 = 0xD13C,
        HOLDING_REG_D13D_PotentiometerCharacteristicPoint_1Y_ParameterSet2 = 0xD13D,
        HOLDING_REG_D13E_PotentiometerCharacteristicPoint_2X_ParameterSet2 = 0xD13E,
        HOLDING_REG_D13F_PotentiometerCharacteristicPoint_2Y_ParameterSet2 = 0xD13F,
        HOLDING_REG_D140_NoFunction = 0xD140,
        HOLDING_REG_D141_NoFunction = 0xD141,
        HOLDING_REG_D142_NoFunction = 0xD142,
        HOLDING_REG_D143_NoFunction = 0xD143,
        HOLDING_REG_D144_NoFunction = 0xD144,
        HOLDING_REG_D145_LimitingSpeedForRunningMonitor = 0xD145,
        HOLDING_REG_D146_Reserved = 0xD146,
        HOLDING_REG_D147_NoFunction = 0xD147,
        HOLDING_REG_D148_NoFunction = 0xD148,
        HOLDING_REG_D149_TransferRate = 0xD149,
        HOLDING_REG_D14A_ParityConfiguration = 0xD14A,
        HOLDING_REG_D14B_NoFunction = 0xD14B,
        HOLDING_REG_D14C_NoFunction = 0xD14C,
        HOLDING_REG_D14D_MotorTemperatureAtStartOfDerating = 0xD14D,
        HOLDING_REG_D14E_MotorTemperatureAtEndOfDerating = 0xD14E,
        HOLDING_REG_D14F_NoFunction = 0xD14F,
        HOLDING_REG_D150_SheddingFunction = 0xD150,
        HOLDING_REG_D151_MaxStartingModulationLevel = 0xD151,
        HOLDING_REG_D152_NumberOfStartAttempts = 0xD152,
        HOLDING_REG_D153_RelayDropoutDelay = 0xD153,
        HOLDING_REG_D154_NoFunction = 0xD154,
        HOLDING_REG_D155_MaximumPower = 0xD155,
        HOLDING_REG_D156_Vacant = 0xD156,
        HOLDING_REG_D157_Vacant = 0xD157,
        HOLDING_REG_D158_Vacant = 0xD158,
        HOLDING_REG_D159_Vacant = 0xD159,
        HOLDING_REG_D15A_Vacant = 0xD15A,
        HOLDING_REG_D15B_FailSafeFunctionRunningDirection = 0xD15B,
        HOLDING_REG_D15C_FailSafeFunctionOnOff = 0xD15C,
        HOLDING_REG_D15D_SetValueForFailsafeFunction = 0xD15D,
        HOLDING_REG_D15E_FailSafeDropoutDelay = 0xD15E,
        HOLDING_REG_D15F_PotentiometerCharacteristicLimitingValueForCableBreak = 0xD15F,
        HOLDING_REG_D160_MinimumSensorValue_MSB = 0xD160,
        HOLDING_REG_D161_MinimumSensorValue_LSB = 0xD161,
        HOLDING_REG_D162_MaximumSensorValue_MSB = 0xD162,
        HOLDING_REG_D163_MaximumSensorValue_LSB = 0xD163,
        HOLDING_REG_D164_SensorUnit_0 = 0xD164,
        HOLDING_REG_D165_SensorUnit_1 = 0xD165,
        HOLDING_REG_D166_SensorUnit_2 = 0xD166,
        HOLDING_REG_D167_SensorUnit_3 = 0xD167,
        HOLDING_REG_D168_SensorUnit_4 = 0xD168,
        HOLDING_REG_D169_SensorUnit_5 = 0xD169,
        HOLDING_REG_D16A_Vacant = 0xD16A,
        HOLDING_REG_D16B_Vacant = 0xD16B,
        HOLDING_REG_D16C_Vacant = 0xD16C,
        HOLDING_REG_D16D_Vacant = 0xD16D,
        HOLDING_REG_D16E_Vacant = 0xD16E,
        HOLDING_REG_D16F_Vacant = 0xD16F,
        HOLDING_REG_D170_CustomerData_0 = 0xD170,
        HOLDING_REG_D171_CustomerData_1 = 0xD171,
        HOLDING_REG_D172_CustomerData_2 = 0xD172,
        HOLDING_REG_D173_CustomerData_3 = 0xD173,
        HOLDING_REG_D174_CustomerData_4 = 0xD174,
        HOLDING_REG_D175_CustomerData_5 = 0xD175,
        HOLDING_REG_D176_CustomerData_6 = 0xD176,
        HOLDING_REG_D177_CustomerData_7 = 0xD177,
        HOLDING_REG_D178_CustomerData_8 = 0xD178,
        HOLDING_REG_D179_CustomerData_9 = 0xD179,
        HOLDING_REG_D17A_CustomerData_A = 0xD17A,
        HOLDING_REG_D17B_CustomerData_B = 0xD17B,
        HOLDING_REG_D17C_CustomerData_C = 0xD17C,
        HOLDING_REG_D17D_CustomerData_D = 0xD17D,
        HOLDING_REG_D17E_CustomerData_E = 0xD17E,
        HOLDING_REG_D17F_CustomerData_F = 0xD17F,
        HOLDING_REG_D180_OperatingHoursCounter = 0xD180,
        HOLDING_REG_D181_OperatingMinutesCounter = 0xD181,
        HOLDING_REG_D182_ErrorIndicator = 0xD182,
        HOLDING_REG_D183_Vacant = 0xD183,
        HOLDING_REG_D184_1stError = 0xD184,
        HOLDING_REG_D185_1stErrorTiming = 0xD185,
        HOLDING_REG_D186_ErrorHistory = 0xD186,
        HOLDING_REG_D19F_ErrorHistoryTiming = 0xD19F,
        HOLDING_REG_D1A0_ReferenceValueOfDClinkVoltage = 0xD1A0,
        HOLDING_REG_D1A1_ReferenceValueOfDClinkCurrent = 0xD1A1,
        HOLDING_REG_D1A2_FanSerialNumber_MSB = 0xD1A2,
        HOLDING_REG_D1A3_FanSerialNumber_LSB = 0xD1A3,
        HOLDING_REG_D1A4_FanProductionDate = 0xD1A4,
        HOLDING_REG_D1A5_FanType_Byte_5 = 0xD1A5,
        HOLDING_REG_D1A6_FanType_Byte_4 = 0xD1A6,
        HOLDING_REG_D1A7_FanType_Byte_3 = 0xD1A7,
        HOLDING_REG_D1A8_FanType_Byte_2 = 0xD1A8,
        HOLDING_REG_D1A9_FanType_Byte_1 = 0xD1A9,
        HOLDING_REG_D1AA_FanType_Byte_0 = 0xD1AA

    } EbmModbusHoldingRegister;

    typedef enum {
        INPUT_REG_D000_Identification = 0xD000,
        INPUT_REG_D001_MaximumNumberOfBytes = 0xD001,
        INPUT_REG_D002_SoftwareNameOfBusController = 0xD002,
        INPUT_REG_D003_SoftwareVersionOfBusController = 0xD003,
        INPUT_REG_D004_SoftwareNameOfCommutationController = 0xD004,
        INPUT_REG_D005_SoftwareVersionOfCommutationController = 0xD005,
        INPUT_REG_D010_ActualSpeed = 0xD010,
        INPUT_REG_D011_MotorStatus = 0xD011,
        INPUT_REG_D012_Warning = 0xD012,
        INPUT_REG_D013_DClinkVoltage = 0xD013,
        INPUT_REG_D014_DClinkCurrent = 0xD014,
        INPUT_REG_D015_ModuleTemperature = 0xD015,
        INPUT_REG_D016_MotorTemperature = 0xD016,
        INPUT_REG_D017_ElectronicsTemperature = 0xD017,
        INPUT_REG_D018_CurrentDirectionOfRotation = 0xD018,
        INPUT_REG_D019_CurrentModulationLevel = 0xD019,
        INPUT_REG_D01A_CurrentSetValue = 0xD01A,
        INPUT_REG_D01B_SensorActualValue = 0xD01B,
        INPUT_REG_D01C_NoFunction = 0xD01C,
        INPUT_REG_D01D_CurrentParameterSet = 0xD01D,
        INPUT_REG_D021_CurrentPower = 0xD021,
        INPUT_REG_D023_SensorActualValue_1 = 0xD023

    } EbmModbusInputRegister;

signals:

private:
    QString m_interface;
    modbus_t *m_bus;

public slots:
    void slot_writeHoldingRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg, quint16 rawdata);
    void slot_readHoldingRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg);
    void slot_readInputRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusInputRegister reg);

signals:
    // Modbus result signals
    void signal_transactionLost(quint64 telegramID);
    void signal_receivedHoldingRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusHoldingRegister reg, quint16 rawdata);
    void signal_receivedInputRegisterData(quint64 telegramID, quint16 adr, EbmModbus::EbmModbusInputRegister reg, quint16 rawdata);
    void signal_wroteHoldingRegisterData(quint64 telegramID);

    // Log output signals
    void signal_newEntry(LogEntry::LoggingCategory loggingCategory, QString module, QString text);
    void signal_entryGone(LogEntry::LoggingCategory loggingCategory, QString module, QString text);
};

#endif // EBMMODBUS_H
