/*
 *  Fintek.h
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 31/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#ifndef _FINTEK_H
#define _FINTEK_H 

#include "SuperIO.h"

// Registers
const UInt8 FINTEK_CONFIGURATION_CONTROL_REGISTER = 0x02;
const UInt8 FINTEK_DEVCIE_SELECT_REGISTER = 0x07;
const UInt8 FINTEK_CHIP_ID_REGISTER = 0x20;
const UInt8 FINTEK_CHIP_REVISION_REGISTER = 0x21;
const UInt8 FINTEK_BASE_ADDRESS_REGISTER = 0x60;

const UInt8 FINTEK_VENDOR_ID_REGISTER = 0x23;
const UInt16 FINTEK_VENDOR_ID = 0x1934;
const UInt8 F71858_HARDWARE_MONITOR_LDN = 0x02;
const UInt8 FINTEK_HARDWARE_MONITOR_LDN = 0x04;

// Hardware Monitor
const UInt8 FINTEK_ADDRESS_REGISTER_OFFSET = 0x05;
const UInt8 FINTEK_DATA_REGISTER_OFFSET = 0x06;

// Hardware Monitor Registers
const UInt8 FINTEK_VOLTAGE_BASE_REG = 0x20;
const UInt8 FINTEK_TEMPERATURE_CONFIG_REG = 0x69;
const UInt8 FINTEK_TEMPERATURE_BASE_REG = 0x70;
const UInt8 FINTEK_FAN_TACHOMETER_REG[] = { 0xA0, 0xB0, 0xC0, 0xD0 };

class Fintek : public SuperIO
{
public:	
	virtual UInt8	ReadByte(UInt8 index);
	virtual SInt16	ReadTemperature(UInt8 index);
	virtual SInt16	ReadVoltage(UInt8 index);
	virtual SInt16	ReadTachometer(UInt8 index);
	
	virtual void	Enter();
	virtual void	Exit();
	
	virtual bool	ProbePort();
	
	virtual void	Start();
};

#endif