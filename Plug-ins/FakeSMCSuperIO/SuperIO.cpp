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
        case F71858: return "Fintek F71858"; break;
        case F71862: return "Fintek F71862"; break;
        case F71869: return "Fintek F71869"; break;
        case F71882: return "Fintek F71882"; break;
        case F71889ED: return "Fintek F71889ED"; break;
        case F71889F: return "Fintek F71889F"; break;
        case IT8512F: return "ITE IT8512F"; break;
        case IT8712F: return "ITE IT8712F"; break;
        case IT8716F: return "ITE IT8716F"; break;
        case IT8718F: return "ITE IT8718F"; break;
        case IT8720F: return "ITE IT8720F"; break;
        case IT8726F: return "ITE IT8726F"; break;
        case IT8752F: return "ITE IT8752F"; break;
        case W83627DHG: return "Winbond W83627DHG"; break;
        case W83627DHGP: return "Winbond W83627DHG-P"; break;
        case W83627EHF: return "Winbond W83627EHF"; break;
        case W83627HF: return "Winbond W83627HF"; break;
        case W83627THF: return "Winbond W83627THF"; break;
        case W83667HG: return "Winbond W83667HG"; break;
        case W83667HGB: return "Winbond W83667HG-B"; break;
        case W83687THF: return "Winbond W83687THF"; break;
			//Slice
        case W83977CTF: return "Winbond W83977CTF"; break;
        case W83977EF: return "Winbond W83977EF"; break;
        case W83977FA: return "Winbond W83977FA"; break;
        case W83977TF: return "Winbond W83977TF"; break;
        case W83977ATF: return "Winbond W83977ATF"; break;
        case W83977AF: return "Winbond W83977AF"; break;
        case W83627SF: return "Winbond W83627SF"; break;
        case W83697HF: return "Winbond W83697HF"; break;
        case W83L517D: return "Winbond W83L517D"; break;
        case W83637HF: return "Winbond W83637HF"; break;
        case W83627UHG: return "Winbond W83627UHG"; break;
        case W83697SF: return "Winbond W83697SF"; break;
        case W83877F:  return "Winbond W83877F"; break;
        case W83877AF: return "Winbond W83877AF"; break;
        case W83877TF: return "Winbond W83877TF"; break;
        case W83877ATF: return "Winbond W83877ATF"; break;
			
		case UnknownSMCS: return "Unknown SMSC (SMSC is not supported now)"; break;
    };
	
	return "Unknown";
}

void SuperIO::LoadConfiguration(IOService* provider)
{
	m_Service = provider;
	
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