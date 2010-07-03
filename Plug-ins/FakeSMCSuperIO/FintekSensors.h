/*
 *  FintekSensors.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Fintek.h"

class FintekTemperatureSensor : public Sensor 
{
public:
	FintekTemperatureSensor(Fintek* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void OnKeyRead(__unused const char* key, char* data)
	{
		data[0] = ((Fintek*)m_Provider)->ReadTemperature(m_Index);
		data[1] = 0;
	};
};

class FintekVoltageSensor : public Sensor 
{
	
public:
	FintekVoltageSensor(Fintek* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void OnKeyRead(__unused const char* key, char* data)
	{
		UInt16 value = fp2e_Encode(((Fintek*)m_Provider)->ReadVoltage(m_Index));
		
		data[0] = (value & 0xff00) >> 8;
		data[1] = value & 0x00ff;
	};
};

class FintekTachometerSensor : public Sensor 
{
public:
	FintekTachometerSensor(Fintek* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void voidOnKeyRead(__unused const char* key, char* data)
	{
		int value = ((Fintek*)m_Provider)->ReadTachometer(m_Index);
		
		if (value > 0)
		{
			data[0] = (value >> 6) & 0xff;
			data[1] = (value << 2) & 0xff;
		}
	};
};
