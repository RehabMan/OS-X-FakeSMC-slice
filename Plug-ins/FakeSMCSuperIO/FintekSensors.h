/*
 *  FintekSensors.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Sensor.h"
#include "Fintek.h"

class FintekTemperatureSensor : public Sensor 
{
public:
	FintekTemperatureSensor(Fintek* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		if (!m_Provider->Lock())
			return kIOReturnSuccess;
		
		m_Value = ((Fintek*)m_Provider)->ReadTemperature(m_Index);
		
		data[0] = m_Value;
		data[1] = 0;
		
		m_Provider->Unlock();
		
		return kIOReturnSuccess;
	};
};

class FintekVoltageSensor : public Sensor 
{
	
public:
	FintekVoltageSensor(Fintek* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		if (!m_Provider->Lock())
			return kIOReturnSuccess;
		
		m_Value = fp2e_Encode(((Fintek*)m_Provider)->ReadVoltage(m_Index));
		
		data[0] = (m_Value & 0xff00) >> 8;
		data[1] = m_Value & 0x00ff;
		
		m_Provider->Unlock();
		
		return kIOReturnSuccess;
	};
};

class FintekTachometerSensor : public Sensor 
{
public:
	FintekTachometerSensor(Fintek* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		if (!m_Provider->Lock())
			return kIOReturnSuccess;
		
		m_Value = ((Fintek*)m_Provider)->ReadTachometer(m_Index);
		
		data[0] = (m_Value >> 6) & 0xff;
		data[1] = (m_Value << 2) & 0xff;
		
		m_Provider->Unlock();
		
		return kIOReturnSuccess;
	};
};
