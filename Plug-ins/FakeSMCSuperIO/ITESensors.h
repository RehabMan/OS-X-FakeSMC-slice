/*
 *  ITESensors.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "ITE.h"
#include "Sensor.h"

class ITETemperatureSensor : public Sensor 
{
public:
	ITETemperatureSensor(ITE* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void OnKeyRead(__unused const char* key, char* data)
	{
		data[0] = ((ITE*)m_Provider)->ReadTemperature(m_Index);
		data[1] = 0;
	};
};

class ITEVoltageSensor : public Sensor 
{
public:
	ITEVoltageSensor(ITE* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
	};
	
	virtual void OnKeyRead(__unused const char* key, char* data)
	{
		UInt16 value = fp2e_Encode(((ITE*)m_Provider)->ReadVoltage(m_Index));
		
		data[0] = (value & 0xff00) >> 8;
		data[1] = value & 0x00ff;
	};
};

class ITETachometerSensor : public Sensor 
{
public:
	ITETachometerSensor(ITE* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
	};
	
	virtual void OnKeyRead(__unused const char* key, char* data)
	{
		UInt16 value = ((ITE*)m_Provider)->ReadTachometer(m_Index);
		
		data[0] = (value >> 6) & 0xff;
		data[1] = (value << 2) & 0xff;
	};
};