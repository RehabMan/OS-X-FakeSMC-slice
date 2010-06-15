/*
 *  WinbondSensors.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Winbond.h"

class WinbondSensor : public Binding 
{
protected:
	Winbond*	m_Provider;
	UInt8		m_Offset;
	
public:
	WinbondSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Binding(key, type, size)
	{
		m_Provider = provider;
		m_Offset = offset;
	};
	
	virtual void	OnKeyRead(__unused const char* key, __unused char* data) {};
	virtual void	OnKeyWrite(__unused const char* key, __unused char* data) {};
};

class WinbondTemperatureSensor : public WinbondSensor 
{
public:
	WinbondTemperatureSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : WinbondSensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class WinbondVoltageSensor : public WinbondSensor 
{
private:
public:
	WinbondVoltageSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : WinbondSensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};

class WinbondTachometerSensor : public WinbondSensor 
{
public:
	WinbondTachometerSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : WinbondSensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void	OnKeyRead(const char* key, char* data);
	virtual void	OnKeyWrite(const char* key, char* data);
};