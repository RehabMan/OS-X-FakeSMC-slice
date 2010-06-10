/*
 *  Fintek.cpp
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 31/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#include "Fintek.h"
#include "fakesmc.h"

UInt8 Fintek::ReadByte(UInt8 reg) 
{
	outb(Address + FINTEK_ADDRESS_REGISTER_OFFSET, reg);
	return inb(Address + FINTEK_DATA_REGISTER_OFFSET);
} 

void Fintek::Enter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x87);
}

void Fintek::Exit()
{
	outb(RegisterPort, 0xAA);
}

short Fintek::ReadTemperature(UInt8 index)
{
	float value;
	
	switch (Model) 
	{
		case F71858: 
		{
			int tableMode = 0x3 & ReadByte(FINTEK_TEMPERATURE_CONFIG_REG);
			int high = ReadByte(FINTEK_TEMPERATURE_BASE_REG + 2 * index);
			int low = ReadByte(FINTEK_TEMPERATURE_BASE_REG + 2 * index + 1);      
			
			if (high != 0xbb && high != 0xcc) 
			{
                int bits = 0;
				
                switch (tableMode) 
				{
					case 0: bits = 0; break;
					case 1: bits = 0; break;
					case 2: bits = (high & 0x80) << 8; break;
					case 3: bits = (low & 0x01) << 15; break;
                }
                bits |= high << 7;
                bits |= (low & 0xe0) >> 1;
				
                short value = (short)(bits & 0xfff0);
				
				return (float)value / 128.0f;
			} 
			else 
			{
                return 0;
			}
		} break;
		default: 
		{
            value = ReadByte(FINTEK_TEMPERATURE_BASE_REG + 2 * (index + 1));
		} break;
	}
	
	return value;
}

bool Fintek::Probe()
{
	Model = UnknownModel;
	
	for (int i = 0; i < 2; i++) 
	{
//		SetPorts(i);
		
		Enter();
		
		UInt8 chipID = ReadByte(SUPERIO_CHIP_ID_REGISTER);
		UInt8 revID = ReadByte(SUPERIO_CHIP_REVISION_REGISTER);

		UInt8 ven1ID = ReadByte(VENDOR_ID_BYTE1_REG);
		UInt8 ven2ID = ReadByte(VENDOR_ID_BYTE2_REG);
		
		IOLog("FakeSMC_SuperIO probe: Fintek chip=%02x %02x %02x %02x\n", chipID,
			  revID,  ven1ID, ven2ID);
		if (((ven1ID<<8)|(ven2ID)) == FINTEK_VENDOR_ID) {
			IOLog("  Fintek chip found but not supported yet, sorry!\n");
			Model = F71858;
			/*
			 F71858 = 0x0507,
			 F71862 = 0x0601, 
			 F71869 = 0x0814,
			 F71882 = 0x0541,
			 F71889ED = 0x0909,
			 F71889F = 0x0723 
			 
			 */
		}
		
	}
	return true;
}

void Fintek::Init()
{
}

void Fintek::Finish()
{
}

void Fintek::Update(const char *key, char *data)
{
	if(CompareKeys(key, "Th0H"))
	{
		short value = ReadTemperature(0);
		data[0] = value & 0xff;
		data[1] = value >> 8;		
	}

}
