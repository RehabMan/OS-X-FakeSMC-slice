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

bool setBoolean(const char * symbol, bool value, OSDictionary * dictionary)
{
	OSBoolean * boolean = value ? kOSBooleanTrue : kOSBooleanFalse;
	
	if (boolean)
	{
		dictionary->setObject(symbol, boolean);
		return true;
	}
	
	return false;
}

bool setArray(const char * symbol, OSArray * value, OSDictionary * dictionary)
{
	dictionary->setObject(symbol, value);
	return true;
}

bool setDictionary(const char * symbol, OSDictionary * value, OSDictionary * dictionary)
{
	dictionary->setObject(symbol, value);
	return true;
}

void SuperIO::Bind(Binding* binding)
{
	binding->Next = m_Binding;
	m_Binding = binding;
}

void SuperIO::FlushBindings()
{
	Binding* iterator = m_Binding;
	
	while (iterator)
	{
		Binding* next = iterator->Next;
		
		delete iterator;
		
		iterator = next;
	}
}

UInt8 SuperIO::ListenPortByte(UInt8 reg)
{
	outb(RegisterPort, reg);
	return inb(ValuePort);
}

UInt16 SuperIO::ListenPortWord(UInt8 reg)
{
	return ((SuperIO::ListenPortByte(reg) << 8) | SuperIO::ListenPortByte(reg + 1));
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
        case IT8712F: return "ITE IT8712F"; break;
        case IT8716F: return "ITE IT8716F"; break;
        case IT8718F: return "ITE IT8718F"; break;
        case IT8720F: return "ITE IT8720F"; break;
        case IT8726F: return "ITE IT8726F"; break;
    };
	
	return "Unknown";
}

void SuperIO::LoadConfiguration(IOService* provider)
{
	m_Service = provider;
	
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