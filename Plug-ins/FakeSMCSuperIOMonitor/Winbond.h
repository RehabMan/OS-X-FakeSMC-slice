/*
 *  Winbond.h
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#ifndef _WINBOND_H
#define _WINBOND_H 

#include "SuperIO.h"

const UInt8 WINBOND_PORTS_COUNT = 5;
const UInt16 WINBOND_PORT[5] = { 0x2e, 0x4e, 0x3f0, 0x370, 0x250};

const UInt8 WINBOND_HARDWARE_MONITOR_LDN	= 0x0B;

const UInt16 WINBOND_VENDOR_ID = 0x5CA3;
const UInt8 WINBOND_HIGH_BYTE = 0x80;

// Winbond Hardware Monitor
const UInt8 WINBOND_ADDRESS_REGISTER_OFFSET = 0x05;
const UInt8 WINBOND_DATA_REGISTER_OFFSET = 0x06;

// Winbond Hardware Monitor Registers
const UInt8 WINBOND_VOLTAGE_BASE_REG = 0x20;
const UInt8 WINBOND_BANK_SELECT_REGISTER = 0x4E;
//const UInt8 WINBOND_VENDOR_ID_REGISTER = 0x4F;
const UInt8 WINBOND_TEMPERATURE_SOURCE_SELECT_REG = 0x49;

//private string[] TEMPERATURE_NAME = 
//new string[] {"CPU", "Auxiliary", "System"};
const UInt8 WINBOND_TEMPERATURE_REG[] = { 0x50, 0x50, 0x27 };
const UInt8 WINBOND_TEMPERATURE_BANK[] = { 1, 2, 0 };

const UInt8 WINBOND_FAN_TACHO_REG[] = { 0x28, 0x29, 0x2A, 0x3F, 0x53 };
const UInt8 WINBOND_FAN_TACHO_BANK[] = { 0, 0, 0, 0, 5 };       
const UInt8 WINBOND_FAN_BIT_REG[] = { 0x47, 0x4B, 0x4C, 0x59, 0x5D };
const UInt8 WINBOND_FAN_DIV_BIT0[] = { 36, 38, 30, 8, 10 };
const UInt8 WINBOND_FAN_DIV_BIT1[] = { 37, 39, 31, 9, 11 };
const UInt8 WINBOND_FAN_DIV_BIT2[] = { 5, 6, 7, 23, 15 };

static UInt16	FanValue[5];
static bool		FanValueObsolete[5];

inline UInt8 Winbond_ReadByte(UInt16 address, UInt8 bank, UInt8 reg) 
{
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	return inb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET));
}

inline void Winbond_WriteByte(UInt16 address, UInt8 bank, UInt8 reg, UInt8 value)
{
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), value); 
}

inline float Winbond_ReadTemperature(UInt16 address, UInt8 index)
{
	UInt32 value = Winbond_ReadByte(address, WINBOND_TEMPERATURE_BANK[index], WINBOND_TEMPERATURE_REG[index]) << 1;
	
	if (WINBOND_TEMPERATURE_BANK[index] > 0) 
		value |= Winbond_ReadByte(address, WINBOND_TEMPERATURE_BANK[index], (UInt8)(WINBOND_TEMPERATURE_REG[index] + 1)) >> 7;
	
	float temperature = (float)value / 2.0f;
	
	return temperature;
}

inline UInt64 Winbond_SetBit(UInt64 target, UInt32 bit, UInt32 value)
{
	if (((value & 1) == value) && bit >= 0 && bit <= 63)
	{
		UInt64 mask = (((UInt64)1) << bit);
		return value > 0 ? target | mask : target & ~mask;
	}
	
	return value;
}

inline UInt16 Winbond_ReadTachometer(UInt16 address, UInt8 index, bool force_update)
{
	if (FanValueObsolete[index] || force_update)
	{
		UInt64 bits = 0;
		
		for (int i = 0; i < 5; i++)
			bits = (bits << 8) | Winbond_ReadByte(address, 0, WINBOND_FAN_BIT_REG[i]);
		
		UInt64 newBits = bits;
		
		for (int i = 0; i < 5; i++)
		{
			int count = Winbond_ReadByte(address, WINBOND_FAN_TACHO_BANK[i], WINBOND_FAN_TACHO_REG[i]);
			
			// assemble fan divisor
			int divisorBits = (int)(
									(((bits >> WINBOND_FAN_DIV_BIT2[i]) & 1) << 2) |
									(((bits >> WINBOND_FAN_DIV_BIT1[i]) & 1) << 1) |
									((bits >> WINBOND_FAN_DIV_BIT0[i]) & 1));
			int divisor = 1 << divisorBits;
			
			FanValue[i] = (count < 0xff) ? 1.35e6f / (float)(count * divisor) : 0;
			FanValueObsolete[i] = false;
			
			// update fan divisor
			if (count > 192 && divisorBits < 7) 
				divisorBits++;
			if (count < 96 && divisorBits > 0)
				divisorBits--;
			
			newBits = Winbond_SetBit(newBits, WINBOND_FAN_DIV_BIT2[i], 
									 (divisorBits >> 2) & 1);
			newBits = Winbond_SetBit(newBits, WINBOND_FAN_DIV_BIT1[i], 
									 (divisorBits >> 1) & 1);
			newBits = Winbond_SetBit(newBits, WINBOND_FAN_DIV_BIT0[i], 
									 divisorBits & 1);
		}
		
		// write new fan divisors 
		for (int i = 4; i >= 0; i--) 
		{
			UInt8 oldByte = (UInt8)(bits & 0xFF);
			UInt8 newByte = (UInt8)(newBits & 0xFF);
			bits = bits >> 8;
			newBits = newBits >> 8;
			if (oldByte != newByte) 
				Winbond_WriteByte(address, 0, WINBOND_FAN_BIT_REG[i], newByte);        
		}
	}
	
	FanValueObsolete[index] = true;
	
	return FanValue[index];
}

// Winbond classes definition

class WinbondTemperatureSensor : public Sensor 
{
public:
	WinbondTemperatureSensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class WinbondVoltageSensor : public Sensor 
{
private:
	ChipModel m_Model;
public:
	WinbondVoltageSensor(UInt16 address, ChipModel model, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
		m_Model = model;
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class WinbondTachometerSensor : public Sensor 
{
public:
	WinbondTachometerSensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class Winbond : public SuperIO
{
private:
protected:
	void	Enter();
	void	Exit();
public:	
	virtual bool	Probe();
	virtual void	Init();
	virtual void	Finish();
};

#endif