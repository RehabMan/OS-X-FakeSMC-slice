/*
 *  Sensor.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 08/07/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _SENSOR_H 
#define _SENSOR_H

//#include "FakeSMCBinding.h"
#include "FakeSMCIntelMonitor.h"

class Sensor : public Binding
{
protected:
	FakeSMCIntelMonitor*	m_Provider;
	UInt8		m_Index;
	char*		m_Key;
	SInt32		m_Value;
public:
	
	Sensor(FakeSMCIntelMonitor* provider, UInt8 index, const char* key, const char* type, UInt8 size)
	{
		m_Provider = provider;
		m_Index = index;
		
		InfoLog("Binding key %s", key);
		
		m_Key = (char*)IOMalloc(5);
		bcopy(key, m_Key, 5);
		
		char* value = (char*)IOMalloc(size);
		FakeSMCAddKey(key, type, size, value, this);
		IOFree(value, size);
	};
	
	~Sensor()
	{
		if (m_Key)
		{
			InfoLog("Removing key %s binding", m_Key);
			FakeSMCRemoveKeyBinding(m_Key);
			IOFree(m_Key, 5);
		}
	}
	
	UInt8			GetIndex() { return m_Index; };
	const char*		GetKey() { return m_Key; };
	SInt32			GetValue() { return m_Value; }
	
	virtual IOReturn OnKeyRead(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
	
	virtual IOReturn OnKeyWrite(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
};

class FrequencySensor : public Sensor
{
public:
	FrequencySensor(FakeSMCIntelMonitor* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)		
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		
		m_Value = ((FakeSMCIntelMonitor*)m_Provider)->Frequency[m_Index];
		data[0]=m_Value>>8;
		data[1]=m_Value&0xff;
		
		return kIOReturnSuccess;
	};
	
};


class TemperatureSensor : public Sensor
{
public:
	TemperatureSensor(FakeSMCIntelMonitor* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)		
	{
		//
	};
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		if (m_Index > MaxCpuCount)
			return kIOReturnNoDevice;
		
		UInt32 magic = 0;		
		mp_rendezvous_no_intrs(IntelThermal, &magic);
		
		data[0]=GlobalThermal[m_Index];
		data[1]=0;
		
		return kIOReturnSuccess;
	};
	
};

class VoltageSensor : public Sensor
{
public:
	VoltageSensor(FakeSMCIntelMonitor* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, char* data)
	{
		m_Value = fp2e_Encode(((FakeSMCIntelMonitor*)m_Provider)->Voltage[m_Index]);
		
		data[0] = (m_Value & 0xff00) >> 8;
		data[1] = m_Value & 0x00ff;
		
		return kIOReturnSuccess;
	};
};

#endif