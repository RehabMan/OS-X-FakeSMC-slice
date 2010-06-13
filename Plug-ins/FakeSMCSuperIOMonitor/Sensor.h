/*
 *  Sensor.h
 *  FakeSMCSuperIOMonitor
 *
 *  Created by Mozodojo on 13/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef Sensor_h
#define Sensor_h

#include "Binding.h"

class Sensor : public Binding
{
protected:
	UInt16		m_Address;
	UInt8		m_Offset;
	
public:
	Sensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size) : Binding(key, type, size)
	{
		m_Address = address;
		m_Offset = offset;
	};
	
	virtual void OnKeyRead(__unused const char* key, __unused char* data)
	{
		// Or it will be link error on kextload
	};
	virtual void OnKeyWrite(__unused const char* key, __unused char* data)
	{
		// Or it will be link error on kextload
	};
};

#endif