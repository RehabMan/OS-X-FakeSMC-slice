/*
 *  WinbondSensors.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Winbond.h"

class WinbondTemperatureSensor : public Sensor 
{
public:
	WinbondTemperatureSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void OnKeyRead(char* data)
	{
		data[0] = ((Winbond*)m_Provider)->ReadTemperature(m_Index);
		data[1] = 0;
	};
};

class WinbondVoltageSensor : public Sensor 
{
public:
	WinbondVoltageSensor(Winbond* provider, UInt8 index, const char* key, const char* type, UInt8 size) : Sensor(provider, index, key, type, size)
	{
		//
	};
	
	virtual void voidOnKeyRead(char* data)
	{
		UInt16 value = fp2e_Encode(((Winbond*)m_Provider)->ReadVoltage(m_Index));
					
		data[0] = (value & 0xff00) >> 8;
		data[1] = (value & 0x00ff);
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
		UInt16 value = ((Winbond*)m_Provider)->ReadTachometer(m_Index, false);
		
		data[0] = (value >> 6) & 0xff;
		data[1] = (value << 2) & 0xff;
	};
};