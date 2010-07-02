/*
 *  WinbondSensors.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Winbond.h"
#include "Sensor.h"

class WinbondTemperatureSensor : public Sensor 
{
public:
	WinbondTemperatureSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void OnKeyRead(__unused const char* key, char* data)
	{
		data[0] = ((Winbond*)m_Provider)->ReadTemperature(m_Index);
		data[1] = 0;
	};
};

class WinbondVoltageSensor : public Sensor 
{
public:
	WinbondVoltageSensor(Winbond* provider, UInt8 index, const char* key, const char* type, UInt8 size) : Sensor(provider, index, key, type, size)
	{
		if (m_Index == 0) 
		{
			InfoLog("Binding key %s", KEY_CPU_VOLTAGE_RAW);
			
			char* value = (char*)IOMalloc(2);
			FakeSMCAddKey(KEY_CPU_VOLTAGE_RAW, TYPE_UI16, 2, value, this);
			IOFree(value, 2);
		}
	};
	
	~WinbondVoltageSensor()
	{
		if (m_Index == 0) 
		{
			InfoLog("Removing key %s binding", KEY_CPU_VOLTAGE_RAW);
			FakeSMCRemoveKeyBinding(KEY_CPU_VOLTAGE_RAW);
		}
	}
	
	virtual void voidOnKeyRead(const char* key, char* data)
	{
		if (CompareKeys(key, KEY_CPU_VOLTAGE_RAW)) 
		{
			UInt16 vcore = ((Winbond*)m_Provider)->GetRawVCore();
			data[0] = (vcore & 0xff00) >> 8;
			data[1] = (vcore & 0x00ff);
		}
		else 
		{
			UInt16 value = fp2e_Encode(((Winbond*)m_Provider)->ReadVoltage(m_Index));
			
			data[0] = (value & 0xff00) >> 8;
			data[1] = (value & 0x00ff);
		}
	};
};

class WinbondTachometerSensor : public Sensor 
{
public:
	WinbondTachometerSensor(Winbond* provider, UInt8 offset, const char* key, const char* type, UInt8 size) : Sensor(provider, offset, key, type, size)
	{
		//
	};
	
	virtual void voidOnKeyRead(__unused const char* key, char* data)
	{
		UInt16 value = ((Winbond*)m_Provider)->ReadTachometer(m_Index, false);
		
		data[0] = (value >> 6) & 0xff;
		data[1] = (value << 2) & 0xff;
	};
};