/*
 *  ITESensors.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "ITE.h"

class ITESensor : public Binding 
{
protected:
	ITE*	m_Provider;
	UInt8	m_Offset;
	
public:
	ITESensor(ITE* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Binding(key, type, size)
	{
		m_Provider = provider;
		m_Offset = offset;
	};
	
	virtual IOReturn	OnKeyRead(__unused const char* key, __unused char* data) {return kIOReturnInvalid;};
	virtual IOReturn	OnKeyWrite(__unused const char* key, __unused char* data) {return kIOReturnInvalid;};
};

class ITETemperatureSensor : public ITESensor 
{
public:
	ITETemperatureSensor(ITE* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : ITESensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn	OnKeyRead(const char* key, char* data);
	virtual IOReturn	OnKeyWrite(const char* key, char* data);
};

class ITEVoltageSensor : public ITESensor 
{
public:
	ITEVoltageSensor(ITE* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : ITESensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn	OnKeyRead(const char* key, char* data);
	virtual IOReturn	OnKeyWrite(const char* key, char* data);
};

class ITETachometerSensor : public ITESensor 
{
public:
	ITETachometerSensor(ITE* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : ITESensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual IOReturn	OnKeyRead(const char* key, char* data);
	virtual IOReturn	OnKeyWrite(const char* key, char* data);
};