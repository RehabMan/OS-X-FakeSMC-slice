/*
 *  Sensor.h
 *  FakeSMCSuperIOMonitor
 *
 *  Created by Mozodojo on 11/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _SENSOR_H 
#define _SENSOR_H

#include <libkern/OSTypes.h>
#include <IOKit/IOLib.h>

#include "FakeSMCPlugin.h"

class Sensor : public FakeSMCPlugin
{
private:
protected:
	UInt16		m_Address;
	UInt8		m_Offset;
	char*		m_Key;
public:
	Sensor*		Next;
	
	Sensor(UInt16 address, UInt8 offset, const char* key, const char* type, UInt8 size)
	{
		InfoLog("Binding key %s", key);
		
		m_Address = address;
		m_Offset = offset;

		m_Key = (char*)IOMalloc(5);
		bcopy(key, m_Key, 5);
		
		char* value = (char*)IOMalloc(size);
		FakeSMCAddKey(key, type, size, value, this);
		IOFree(value, size);
	};
	
	~Sensor()
	{
		InfoLog("Removing key %s binding", m_Key);
		
		IOFree(m_Key, 5);
		FakeSMCRemoveKeyBinding(m_Key);
	};
	
	const char* GetKey() { return m_Key; }
	
	virtual void OnKeyRead(const char* key, char* data);
	virtual void OnKeyWrite(const char* key, char* data);
};

#endif