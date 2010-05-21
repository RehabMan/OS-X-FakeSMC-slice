/*
 *
 *  Copyright © 2010 mozodojo. All rights reserved.
 *
 *	This code includes parts of original Open Hardware Monitor code
 *	Copyright © 2010 Michael Möller. All Rights Reserved.
 *
 */

#include <architecture/i386/pio.h>

#include "FakeSMCLPCMonitor.h"



static void Update(SMCData node)
{
	/*UInt32 magic = 0;
	
	mp_rendezvous_no_intrs(IntelThermal, &magic);
	
	UInt8 cpun = node->key[2] - 48;
	
	if(CpuCoreiX)
	{
		node->data[0] = TjMaxCoreiX[cpun] - Thermal[cpun];
	}
	else 
	{
		node->data[0] = TjMax - Thermal[cpun];
	}
	
	node->data[1] = 0;*/
}

#define super IOService
OSDefineMetaClassAndStructors(LPCMonitorPlugin, IOService)

UInt8 LPCMonitorPlugin::ReadByte(UInt8 reg)
{
	return inb(reg);
}

UInt16 LPCMonitorPlugin::ReadWord(UInt8 reg)
{
	return (UInt16)((ReadByte(reg) << 8) | ReadByte((UInt8)(reg + 1)));
}

void LPCMonitorPlugin::Select(UInt8 logicalDeviceNumber)
{
	outb(RegisterPort, DEVCIE_SELECT_REGISTER);
	outb(ValuePort, logicalDeviceNumber);
}

void LPCMonitorPlugin::IT87Enter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x01);
	outb(RegisterPort, 0x55);
	outb(RegisterPort, 0x55);
}

void LPCMonitorPlugin::IT87Exit()
{
	outb(RegisterPort, CONFIGURATION_CONTROL_REGISTER);
	outb(ValuePort, 0x02);
}

void LPCMonitorPlugin::WinbondFintekEnter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x87);
}

void LPCMonitorPlugin::WinbondFintekExit()
{
	outb(RegisterPort, 0xAA);      
}

IOService* LPCMonitorPlugin::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) return 0;
	
	switch (ChipModel) 
	{
        case F71858: ChipName = "Fintek F71858"; break;
        case F71862: ChipName = "Fintek F71862"; break;
        case F71869: ChipName = "Fintek F71869"; break;
        case F71882: ChipName = "Fintek F71882"; break;
        case F71889ED: ChipName = "Fintek F71889ED"; break;
        case F71889F: ChipName = "Fintek F71889F"; break;
        case IT8712F: ChipName = "ITE IT8712F"; break;
        case IT8716F: ChipName = "ITE IT8716F"; break;
        case IT8718F: ChipName = "ITE IT8718F"; break;
        case IT8720F: ChipName = "ITE IT8720F"; break;
        case IT8726F: ChipName = "ITE IT8726F"; break;
        case W83627DHG: ChipName = "Winbond W83627DHG"; break;
        case W83627DHGP: ChipName = "Winbond W83627DHG-P"; break;
        case W83627EHF: ChipName = "Winbond W83627EHF"; break;
        case W83627HF: ChipName = "Winbond W83627HF"; break;
        case W83627THF: ChipName = "Winbond W83627THF"; break;
        case W83667HG: ChipName = "Winbond W83667HG"; break;
        case W83667HGB: ChipName = "Winbond W83667HG-B"; break;
        case W83687THF: ChipName = "Winbond W83687THF"; break;
    };
	
	return this;
}

bool LPCMonitorPlugin::start(IOService * provider)
{
	if (!super::start(provider)) return false;
	
	registerService(0);
	
	return true;	
}

bool LPCMonitorPlugin::init(OSDictionary *properties)
{
    super::init(properties);
	
	return true;
}

void LPCMonitorPlugin::stop (IOService* provider)
{
	super::stop(provider);
}

void LPCMonitorPlugin::free ()
{
	super::free ();
}