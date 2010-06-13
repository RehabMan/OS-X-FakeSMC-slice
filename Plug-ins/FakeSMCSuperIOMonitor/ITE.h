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
#include "Sensor.h"

const UInt8 ITE_PORTS_COUNT = 4;
const UInt16 ITE_PORT[] = { 0x2e, 0x4e, 0x290, 0x370 };

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
const UInt8 ITE_FAN_FORCE_PWM_REG[5] = { 0x17, 0x16, 0x15, 0x88, 0x89 };
const UInt8 ITE_START_PWM_VALUE_REG[5] = { 0x73, 0x6b, 0x63, 0x9b, 0x93 };
const UInt8 ITE_VOLTAGE_BASE_REG = 0x20;

const float ITE_VOLTAGE_GAIN[] = {1, 1, 1, (6.8f / 10 + 1), 1, 1, 1, 1, 1 };

// ITE base functions definition

inline UInt8 ITE_ReadByte(UInt16 address, UInt8 reg, bool* valid)
{
	outb(address + ITE_ADDRESS_REGISTER_OFFSET, reg);
	
	UInt8 value = inb(address + ITE_DATA_REGISTER_OFFSET);
	valid = (bool*)(reg == inb(address + ITE_DATA_REGISTER_OFFSET));
	
	return value;
}

inline UInt8 ITE_ReadTemperature(UInt16 address, UInt8 index, bool* valid)
{
	return ITE_ReadByte(address, ITE_TEMPERATURE_BASE_REG + index, valid);
}

inline UInt16 ITE_ReadTachometer(UInt16 address, UInt8 index, bool* valid)
{
	int value = ITE_ReadByte(address, ITE_FAN_TACHOMETER_REG[index], valid);
	
	if(valid)
	{
		value |= ITE_ReadByte(address, ITE_FAN_TACHOMETER_EXT_REG[index], valid) << 8;
		value = valid && value > 0x3f && value < 0xffff ? (float)(1350000 + value) / (float)(value * 2) : 0;
	}
	
	return value;
}

// ITE classes definition

class ITETemperatureSensor : public Sensor 
{
public:
	ITETemperatureSensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
		
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class ITEVoltageSensor : public Sensor 
{
public:
	ITEVoltageSensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class ITETachometerSensor : public Sensor 
{
public:
	ITETachometerSensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class ITE : public SuperIO
{
protected:
	void	Enter();
	void	Exit();
public:	
	bool			IsFanControlled() { return m_FanControl; };
	
	virtual bool	Probe();
	virtual void	Init();
	virtual void	Finish();
};

#endif