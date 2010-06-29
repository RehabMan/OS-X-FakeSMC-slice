/*
 *  WinbondSensors.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Winbond.h"
#include "Sensor.h"

class WinbondTemperatureSensor : public Sensor 
{
public:
	WinbondTemperatureSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void OnKeyRead(__unused const char* key, char* data)
	{
		data[0] = m_Provider->ReadTemperature(0);
		data[1] = 0;
	};
};

class WinbondVoltageSensor : public Sensor 
{
private:
public:
	WinbondVoltageSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void voidOnKeyRead(__unused const char* key, char* data)
	{
		UInt16 value = fp2e_Encode(m_Provider->ReadVoltage(m_Index));
		
		data[0] = (value & 0xff00) >> 8;
		data[1] = value & 0x00ff;
	};
};

class WinbondTachometerSensor : public Sensor 
{
public:
	WinbondTachometerSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void voidOnKeyRead(__unused const char* key, char* data)
	{
		UInt16 value = m_Provider->ReadTachometer(m_Index, false);
		
		data[0] = (value >> 6) & 0xff;
		data[1] = (value << 2) & 0xff;
	};
};