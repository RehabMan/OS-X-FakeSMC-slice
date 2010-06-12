/*
 *  IT87x.h
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
const UInt8 ITE_FAN_TACHOMETER_REG[] = { 0x0d, 0x0e, 0x0f, 0x80, 0x82 };
const UInt8 ITE_FAN_TACHOMETER_EXT_REG[] = { 0x18, 0x19, 0x1a, 0x81, 0x83 };
const UInt8 ITE_FAN_FORCE_PWM_REG[] = { 0x17, 0x16, 0x15 };
const UInt8 ITE_START_PWM_VALUE_REG[] = { 0x73, 0x6b, 0x63 };
const UInt8 ITE_VOLTAGE_BASE_REG = 0x20;

const float ITE_VOLTAGE_GAIN[] = {1, 1, 1, (6.8f / 10 + 1), 1, 1, 1, 1, 1 };

// ITE base functions definition

inline UInt8 IT87x_ReadByte(UInt16 address, UInt8 reg, bool* valid)
{
	outb(address + ITE_ADDRESS_REGISTER_OFFSET, reg);
	
	UInt8 value = inb(address + ITE_DATA_REGISTER_OFFSET);
	valid = (bool*)(reg == inb(address + ITE_DATA_REGISTER_OFFSET));
	
	return value;
}

inline UInt8 IT87x_ReadTemperature(UInt16 address, UInt8 index, bool* valid)
{
	return IT87x_ReadByte(address, ITE_TEMPERATURE_BASE_REG + index, valid);
}

inline UInt16 IT87x_ReadTachometer(UInt16 address, UInt8 index, bool* valid)
{
	int value = IT87x_ReadByte(address, ITE_FAN_TACHOMETER_REG[index], valid);
	
	if(valid)
	{
		value |= IT87x_ReadByte(address, ITE_FAN_TACHOMETER_EXT_REG[index], valid) << 8;
		value = valid && value > 0x3f && value < 0xffff ? (float)(1350000 + value) / (float)(value * 2) : 0;
	}
	
	return value;
}

// ITE classes definition

class IT87xTemperatureSensor : public Sensor 
{
public:
	IT87xTemperatureSensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
		
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class IT87xVoltageSensor : public Sensor 
{
public:
	IT87xVoltageSensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class IT87xTachometerSensor : public Sensor 
{
public:
	IT87xTachometerSensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(address, offset, key, type, size)
	{
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class IT87xTachometerController : public Binding 
{
private:
	UInt16	m_Address;
	UInt8	m_Offset;
	UInt8	m_Index;

	int		m_MinRpm;
	int		m_MaxRpm;
	
	void	ForcePWM(UInt16 slope);
	
public:
	IT87xTachometerController(UInt16 address, UInt8 offset, UInt8 index)
	{
		m_Address = address;
		m_Offset = offset;
		m_Index = index;
		
		bool* valid;
		
		//Back up temperature sensor selection
		UInt8 TempBackup = IT87x_ReadByte(m_Address, ITE_FAN_FORCE_PWM_REG[m_Offset], valid);
		
		if (valid)
		{
			char tmpKey[5];
			char value[2];
			
			//Determine maximum speed
			ForcePWM(0x7f);
			
			IOSleep(5000);
			
			m_MaxRpm = (IT87x_ReadTachometer(m_Address, m_Offset, valid) / 50 + 1) * 50;
			
			value[0] = (m_MaxRpm << 2) >> 8;
			value[1] = (m_MaxRpm << 2) & 0xff;
			
			//Determine minimum speed
			ForcePWM(0);
			
			IOSleep(5000);
			
			m_MinRpm = (IT87x_ReadTachometer(m_Address, m_Offset, valid) / 50) * 50;
			
			if (m_MaxRpm - m_MinRpm > 50)
			{
				snprintf(tmpKey, 5, "F%dMx", m_Index);
				FakeSMCAddKey(tmpKey, "fpe2", 2, value);
				
				value[0] = (m_MinRpm << 2) >> 8;
				value[1] = (m_MinRpm << 2) & 0xff;
				
				DebugLog("Fan #%d MIN=%drpm MAX=%drpm", m_Offset, m_MinRpm, m_MaxRpm);
				
				
									
				m_Key = (char*)IOMalloc(5);
				snprintf(m_Key, 5, "F%dMn", m_Index);
				
				InfoLog("Binding key %s", m_Key);
				
				FakeSMCAddKey(m_Key, "fpe2", 2, value, this);
			}
			
			//Restore temperature sensor selection
			ForcePWM(TempBackup);
			
			IOSleep(1000);
		}
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};


class IT87x : public SuperIO
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