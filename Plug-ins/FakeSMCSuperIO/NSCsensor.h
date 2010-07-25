/*
 *  NSCsensor.h
 *  FakeSMCSuperIO
 *
 *  Created by Slice on 25.07.10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Sensor.h"
#include "NSC.h"

class NSCTemperatureSensor : public Sensor 
{
public:
	NSCTemperatureSensor(NSC* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		if (!m_Provider->Lock())
			return kIOReturnSuccess;
		
		m_Value = ((NSC*)m_Provider)->ReadTemperature(m_Index);
		
		data[0] = m_Value;
		data[1] = 0;
		
		m_Provider->Unlock();
		
		return kIOReturnSuccess;
	};
};

class NSCTachometerSensor : public Sensor 
{
public:
	NSCTachometerSensor(NSC* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		if (!m_Provider->Lock())
			return kIOReturnSuccess;
		
		m_Value = ((NSC*)m_Provider)->ReadTachometer(m_Index);
		
		data[0] = (m_Value >> 6) & 0xff;
		data[1] = (m_Value << 2) & 0xff;
		
		m_Provider->Unlock();
		
		return kIOReturnSuccess;
	};
};
