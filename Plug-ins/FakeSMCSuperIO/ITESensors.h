/*
 *  ITESensors.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Sensor.h"
#include "ITE.h"

class ITETemperatureSensor : public Sensor 
{
public:
	ITETemperatureSensor(ITE* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		m_Value = ((ITE*)m_Provider)->ReadTemperature(m_Index);
		
		data[0] = m_Value;
		data[1] = 0;
		
		return kIOReturnSuccess;
	};
};

class ITEVoltageSensor : public Sensor 
{
public:
	ITEVoltageSensor(ITE* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		m_Value = fp2e_Encode(((ITE*)m_Provider)->ReadVoltage(m_Index));
		
		data[0] = (m_Value & 0xff00) >> 8;
		data[1] = m_Value & 0x00ff;
		
		return kIOReturnSuccess;
	};
};

class ITETachometerSensor : public Sensor 
{
public:
	ITETachometerSensor(ITE* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		m_Value = ((ITE*)m_Provider)->ReadTachometer(m_Index);
		
		data[0] = (m_Value >> 6) & 0xff;
		data[1] = (m_Value << 2) & 0xff;
		
		return kIOReturnSuccess;
	};
};