/*
 *  SuperIO.cpp
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#include "SuperIO.h"

void SuperIO::RegisterSensor(Sensor* sensor)
{
	sensor->Next = m_Sensor;
	m_Sensor = sensor;
}

void SuperIO::FlushSensors()
{
	Sensor* sensor = m_Sensor;
	
	while (sensor) 
	{
		Sensor* next = sensor->Next;
		
		delete sensor;
		
		sensor = next;
	}
}

UInt8 SuperIO::ReadByte(UInt8 reg)
{
	outb(RegisterPort, reg);
	return inb(ValuePort);
}

UInt16 SuperIO::ReadWord(UInt8 reg)
{
	return ((SuperIO::ReadByte(reg) << 8) | SuperIO::ReadByte(reg + 1));
}

void SuperIO::Select(UInt8 logicalDeviceNumber)
{
	outb(RegisterPort, SUPERIO_DEVCIE_SELECT_REGISTER);
	outb(ValuePort, logicalDeviceNumber);
}

const char* SuperIO::GetModelName()
{
	switch (Model) 
	{
        case F71858: return "Fintek F71858"; break;
        case F71862: return "Fintek F71862"; break;
        case F71869: return "Fintek F71869"; break;
        case F71882: return "Fintek F71882"; break;
        case F71889ED: return "Fintek F71889ED"; break;
        case F71889F: return "Fintek F71889F"; break;
        case IT8712F: return "ITE IT8712F"; break;
        case IT8716F: return "ITE IT8716F"; break;
        case IT8718F: return "ITE IT8718F"; break;
        case IT8720F: return "ITE IT8720F"; break;
        case IT8726F: return "ITE IT8726F"; break;
        case W83627DHG: return "Winbond W83627DHG"; break;
        case W83627DHGP: return "Winbond W83627DHG-P"; break;
        case W83627EHF: return "Winbond W83627EHF"; break;
        case W83627HF: return "Winbond W83627HF"; break;
        case W83627THF: return "Winbond W83627THF"; break;
        case W83667HG: return "Winbond W83667HG"; break;
        case W83667HGB: return "Winbond W83667HG-B"; break;
        case W83687THF: return "Winbond W83687THF"; break;
    };
	
	return "Unknown";
}

void SuperIO::LoadConfiguration(IOService* provider)
{
	OSBoolean* fanControl = OSDynamicCast(OSBoolean, provider->getProperty("Enable Fan Control"));
	
	m_FanControl = fanControl->getValue();
	
	OSArray* fanIDs = OSDynamicCast(OSArray, provider->getProperty("Fan Names"));
	
	if (fanIDs) fanIDs = OSArray::withArray(fanIDs);
	
    if (fanIDs) 
	{
		UInt32 count = fanIDs->getCount();
		
		if(count > 5) count = 5;
		
		for (UInt32 i = 0; i < count; i++)
		{
			OSString* name = OSDynamicCast(OSString, fanIDs->getObject(i)); 
			FanName[i] = name->getCStringNoCopy();
		}
		
		fanIDs->release();
    }
}