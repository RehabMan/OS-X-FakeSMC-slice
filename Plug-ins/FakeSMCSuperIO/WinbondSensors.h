/*
 *  WinbondSensors.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Sensor.h"
#include "Winbond.h"

class WinbondTemperatureSensor : public Sensor 
{
public:
	WinbondTemperatureSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		if (!m_Provider->Lock())
			return kIOReturnSuccess;
		
		m_Value = ((Winbond*)m_Provider)->ReadTemperature(m_Index);
		
		data[0] = m_Value;
		data[1] = 0;
		
		m_Provider->Unlock();
		
		return kIOReturnSuccess;
	};
};

class WinbondVoltageSensor : public Sensor 
{
public:
	WinbondVoltageSensor(Winbond* provider, UInt8 index, const char* key, const char* type, UInt8 size) : Sensor(provider, index, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		if (!m_Provider->Lock())
			return kIOReturnSuccess;
		
		m_Value = fp2e_Encode(((Winbond*)m_Provider)->ReadVoltage(m_Index));
					
		data[0] = (m_Value & 0xff00) >> 8;
		data[1] = (m_Value & 0x00ff);
		
		m_Provider->Unlock();
		
		return kIOReturnSuccess;
	};
};

class WinbondTachometerSensor : public Sensor 
{
public:
	WinbondTachometerSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		if (!m_Provider->Lock())
			return kIOReturnSuccess;
		
		m_Value = ((Winbond*)m_Provider)->ReadTachometer(m_Index, false);
		
		data[0] = (m_Value >> 6) & 0xff;
		data[1] = (m_Value << 2) & 0xff;
		
		m_Provider->Unlock();
		
		return kIOReturnSuccess;
	};
};