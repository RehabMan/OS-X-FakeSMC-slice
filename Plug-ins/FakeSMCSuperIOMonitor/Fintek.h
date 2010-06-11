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

const UInt8 FINTEK_PORTS_COUNT = 2;
const UInt16 FINTEK_PORT[2] = { 0x2e, 0x4e };

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

inline UInt8 Fintek_ReadByte(UInt16 address, UInt8 reg) 
{
	outb(address + FINTEK_ADDRESS_REGISTER_OFFSET, reg);
	return inb(address + FINTEK_DATA_REGISTER_OFFSET);
} 

inline short Fintek_ReadTemperature(UInt16 address, ChipModel model, UInt8 index)
{
	float value;
	
	switch (model) 
	{
		case F71858: 
		{
			int tableMode = 0x3 & Fintek_ReadByte(address, FINTEK_TEMPERATURE_CONFIG_REG);
			int high = Fintek_ReadByte(address, FINTEK_TEMPERATURE_BASE_REG + 2 * index);
			int low = Fintek_ReadByte(address, FINTEK_TEMPERATURE_BASE_REG + 2 * index + 1);      
			
			if (high != 0xbb && high != 0xcc) 
			{
                int bits = 0;
				
                switch (tableMode) 
				{
					case 0: bits = 0; break;
					case 1: bits = 0; break;
					case 2: bits = (high & 0x80) << 8; break;
					case 3: bits = (low & 0x01) << 15; break;
                }
                bits |= high << 7;
                bits |= (low & 0xe0) >> 1;
				
                short value = (short)(bits & 0xfff0);
				
				return (float)value / 128.0f;
			} 
			else 
			{
                return 0;
			}
		} break;
		default: 
		{
            value = Fintek_ReadByte(address, FINTEK_TEMPERATURE_BASE_REG + 2 * (index + 1));
		} break;
	}
	
	return value;
}

inline UInt16 Fintek_ReadTachometer(UInt16 address, UInt8 index)
{
	int value = Fintek_ReadByte(address, FINTEK_FAN_TACHOMETER_REG[index]) << 8;
	value |= Fintek_ReadByte(address, FINTEK_FAN_TACHOMETER_REG[index] + 1);
	
	if (value > 0)
		value = (value < 0x0fff) ? 1.5e6f / value : 0;
	
	return value;
}

// Fintek classes definition


class FintekTemperatureSensor : public Sensor 
{
private:
	ChipModel m_Model;
public:
	FintekTemperatureSensor(UInt16 address, ChipModel model, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
		m_Model = model;
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class FintekVoltageSensor : public Sensor 
{

public:
	FintekVoltageSensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class FintekTachometerSensor : public Sensor 
{
public:
	FintekTachometerSensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class Fintek : public SuperIO
{
protected:
	void	Enter();
	void	Exit();
public:	
	virtual bool	Probe();
	virtual void	Init();
	virtual void	Finish();
};

#endif