/*
 *  FintekSensors.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Fintek.h"

class FintekSensor : public Binding 
{
protected:
	Fintek*	m_Provider;
	UInt8	m_Offset;
	
public:
	FintekSensor(Fintek* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Binding(key, type, size)
	{
		m_Provider = provider;
		m_Offset = offset;
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class FintekTemperatureSensor : public FintekSensor 
{
public:
	FintekTemperatureSensor(Fintek* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : FintekSensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class FintekVoltageSensor : public FintekSensor 
{
	
public:
	FintekVoltageSensor(Fintek* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : FintekSensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class FintekTachometerSensor : public FintekSensor 
{
public:
	FintekTachometerSensor(Fintek* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : FintekSensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};
