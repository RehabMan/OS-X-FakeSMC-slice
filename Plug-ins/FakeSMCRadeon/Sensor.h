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

#include "FakeSMCBinding.h"
#include "ATICard.h"

class Sensor : public Binding
{
protected:
	ATICard*	m_Provider;
	UInt8		m_Index;
	char*		m_Key;
	SInt32		m_Value;
public:
	
	Sensor(ATICard* provider, UInt8 index, const char* key, const char* type, UInt8 size)
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


class R5xxTemperatureSensor : public Sensor {
public:
	R5xxTemperatureSensor(const char* key, const char* type, UInt8 size) : Binding(key, type, size) {};
	virtual IOReturn OnKeyRead(const char* key, char* data);
};

class R6xxTemperatureSensor : public Sensor {
public:
	R6xxTemperatureSensor(const char* key, const char* type, UInt8 size) : Binding(key, type, size) {};
	virtual IOReturn OnKeyRead(const char* key, char* data);
};

class R7xxTemperatureSensor : public Sensor {
public:
	R7xxTemperatureSensor(const char* key, const char* type, UInt8 size) : Binding(key, type, size) {};
	virtual IOReturn OnKeyRead(const char* key, char* data);
};

class EverTemperatureSensor : public Sensor {
public:
	EverTemperatureSensor(const char* key, const char* type, UInt8 size) : Binding(key, type, size) {};
	virtual IOReturn OnKeyRead(const char* key, char* data);
};

class FanSensor : public Sensor {
public:
	FanSensor(const char* key, const char* type, UInt8 size) : Binding(key, type, size) {};
	virtual IOReturn OnKeyRead(const char* key, char* data);
};




#endif