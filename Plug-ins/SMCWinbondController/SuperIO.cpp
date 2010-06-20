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
        case W83627DHG: return "Winbond W83627DHG";
        case W83627DHGP: return "Winbond W83627DHG-P";
        case W83627EHF: return "Winbond W83627EHF";
        case W83627HF: return "Winbond W83627HF";
        case W83627THF: return "Winbond W83627THF";
        case W83667HG: return "Winbond W83667HG";
        case W83667HGB: return "Winbond W83667HG-B";
        case W83687THF: return "Winbond W83687THF";
			//Slice
        case W83977CTF: return "Winbond W83977CTF";
        case W83977EF: return "Winbond W83977EF";
        case W83977FA: return "Winbond W83977FA";
        case W83977TF: return "Winbond W83977TF";
        case W83977ATF: return "Winbond W83977ATF";
        case W83977AF: return "Winbond W83977AF";
        case W83627SF: return "Winbond W83627SF";
        case W83697HF: return "Winbond W83697HF";
        case W83L517D: return "Winbond W83L517D";
        case W83637HF: return "Winbond W83637HF";
        case W83627UHG: return "Winbond W83627UHG";
        case W83697SF: return "Winbond W83697SF";
        case W83877F:  return "Winbond W83877F";
        case W83877AF: return "Winbond W83877AF";
        case W83877TF: return "Winbond W83877TF";
        case W83877ATF: return "Winbond W83877ATF";			
    };
	
	return "Unknown";
}

void SuperIO::LoadConfiguration(IOService* provider)
{
	m_Service = provider;
	
	OSBoolean* fanControl = OSDynamicCast(OSBoolean, provider->getProperty("Enable Fan Control"));
	
	if (fanControl)
		m_FanControl = fanControl->getValue();
	
	OSArray* fanIDs = OSDynamicCast(OSArray, provider->getProperty("Fan Names"));
	
	if (fanIDs) 
		fanIDs = OSArray::withArray(fanIDs);
	
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