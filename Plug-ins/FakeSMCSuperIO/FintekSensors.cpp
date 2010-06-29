/*
 *  FintekSensors.cpp
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "FintekSensors.h"

void FintekTemperatureSensor::OnKeyRead(__unused const char* key, char* data)
{
	data[0] = m_Provider->ReadTemperature(m_Index);
	data[1] = 0;
}

void FintekTemperatureSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void FintekVoltageSensor::OnKeyRead(__unused const char* key, char* data)
{
	UInt16 value = fp2e_Encode(m_Provider->ReadVoltage(m_Index));
	
	data[0] = (value & 0xff00) >> 8;
	data[1] = value & 0x00ff;
}

void FintekVoltageSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}

void FintekTachometerSensor::OnKeyRead(__unused const char* key, char* data)
{
	int value = m_Provider->ReadTachometer(m_Index);
	
	if (value > 0)
	{
		data[0] = (value >> 6) & 0xff;
		data[1] = (value << 2) & 0xff;
	}
}

void FintekTachometerSensor::OnKeyWrite(__unused const char* key, __unused char* data)
{
	
}
