/*
 *  ITE.h
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#ifndef _IT87X_H 
#define _IT87X_H

#include "SuperIO.h"

const UInt8 ITE_PORTS_COUNT = 5;
const UInt16 ITE_PORT[] = { 0x2e, 0x4e, 0x25e, 0x290, 0x370 };

const UInt8 IT87_ENVIRONMENT_CONTROLLER_LDN = 0x04;

// ITE
const UInt8 ITE_VENDOR_ID = 0x90;

// ITE Environment Controller
const UInt8 ITE_ADDRESS_REGISTER_OFFSET = 0x05;
const UInt8 ITE_DATA_REGISTER_OFFSET = 0x06;

// ITE Environment Controller Registers    
const UInt8 ITE_CONFIGURATION_REGISTER = 0x00;
const UInt8 ITE_TEMPERATURE_BASE_REG = 0x29;
const UInt8 ITE_VENDOR_ID_REGISTER = 0x58;
const UInt8 ITE_FAN_TACHOMETER_16_BIT_ENABLE_REGISTER = 0x0c;
const UInt8 ITE_FAN_TACHOMETER_REG[5] = { 0x0d, 0x0e, 0x0f, 0x80, 0x82 };
const UInt8 ITE_FAN_TACHOMETER_EXT_REG[5] = { 0x18, 0x19, 0x1a, 0x81, 0x83 };
const UInt8 ITE_VOLTAGE_BASE_REG = 0x20;

const float ITE_VOLTAGE_GAIN[] = {1, 1, 1, (6.8f / 10 + 1), 1, 1, 1, 1, 1 };

const UInt8 ITE_SMARTGUARDIAN_FORCE_PWM[5]				= { 0x15, 0x16, 0x17, 0x88, 0x89 };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_STOP[5]		= { 0x60, 0x68, 0x70, 0x90, 0x98 };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_START[5]		= { 0x61, 0x69, 0x71, 0x91, 0x99 };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_FULL_ON[5]	= { 0x62, 0x6a, 0x72, 0x92, 0x9a };
const UInt8 ITE_SMARTGUARDIAN_START_PWM[5]				= { 0x63, 0x6b, 0x73, 0x93, 0x9b };
const UInt8 ITE_SMARTGUARDIAN_CONTROL[5]				= { 0x64, 0x6c, 0x74, 0x94, 0x9c };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_FULL_OFF[5]	= { 0x65, 0x6d, 0x75, 0x95, 0x9d };

class ITE : public SuperIO
{
protected:
	void	Enter();
	void	Exit();
public:	
	bool			IsFanControlled() { return m_FanControl; };
	void			WriteByte(UInt8 reg, UInt8 value);
	
	virtual UInt8	ReadByte(UInt8 index, bool* valid);
	virtual UInt16	ReadWord(UInt8 index1, UInt8 index2, bool* valid);
	virtual SInt16	ReadTemperature(UInt8 index);
	virtual SInt16	ReadVoltage(UInt8 index);
	virtual SInt16	ReadTachometer(UInt8 index);
	
	virtual bool	Probe();
	virtual void	Init();
	virtual void	Finish();
};

#endif