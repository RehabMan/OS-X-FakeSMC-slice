/*
 *  Winbond.cpp
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "Winbond.h"
#include "fakesmc.h"

Winbond* WinbondInstance;

static void Update(const char* key, char* data)
{
	WinbondInstance->Update(key, data);
}

void Winbond::Enter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x01);
	outb(RegisterPort, 0x55);
	outb(RegisterPort, 0x55);
}

void Winbond::Exit()
{
	outb(RegisterPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	outb(ValuePort, 0x02);
}

UInt8 Winbond::ReadByte(UInt8 bank, UInt8 reg) 
{
	outb((UInt16)(Address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(Address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(Address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	return inb((UInt16)(Address + WINBOND_DATA_REGISTER_OFFSET));
}

void Winbond::WriteByte(UInt8 bank, UInt8 reg, UInt8 value)
{
	outb((UInt16)(Address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(Address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(Address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	outb((UInt16)(Address + WINBOND_DATA_REGISTER_OFFSET), value); 
}

short Winbond::ReadTemperature(UInt8 index)
{
	UInt32 value = ReadByte(WINBOND_TEMPERATURE_BANK[index], WINBOND_TEMPERATURE_REG[index]) << 1;
	
	if (WINBOND_TEMPERATURE_BANK[index] > 0) 
		value |= ReadByte(WINBOND_TEMPERATURE_BANK[index], (UInt8)(WINBOND_TEMPERATURE_REG[index] + 1)) >> 7;
	
	float temperature = (float)value / 2.0f;
	
	return temperature;
}

UInt64 Winbond::SetBit(UInt64 target, UInt32 bit, UInt32 value)
{
	if (((value & 1) == value) && bit >= 0 && bit <= 63)
	{
		UInt64 mask = (((UInt64)1) << bit);
		return value > 0 ? target | mask : target & ~mask;
	}
	
	return value;
}

void Winbond::UpdateRPM()
{
	UInt64 bits = 0;
	
	for (int i = 0; i < 5; i++)
		bits = (bits << 8) | ReadByte(0, WINBOND_FAN_BIT_REG[i]);
	
	UInt64 newBits = bits;
	
	for (int i = 0; i < 5; i++)
	{
		int count = ReadByte(WINBOND_FAN_TACHO_BANK[i], WINBOND_FAN_TACHO_REG[i]);
		
		// assemble fan divisor
		int divisorBits = (int)(
								(((bits >> WINBOND_FAN_DIV_BIT2[i]) & 1) << 2) |
								(((bits >> WINBOND_FAN_DIV_BIT1[i]) & 1) << 1) |
								((bits >> WINBOND_FAN_DIV_BIT0[i]) & 1));
		int divisor = 1 << divisorBits;
		
		FanValue[i] = (count < 0xff) ? 1.35e6f / (float)(count * divisor) : 0;
		FanValueObsolete[i] = false;
		
		// update fan divisor
		if (count > 192 && divisorBits < 7) 
			divisorBits++;
		if (count < 96 && divisorBits > 0)
			divisorBits--;
		
		newBits = SetBit(newBits, WINBOND_FAN_DIV_BIT2[i], 
								(divisorBits >> 2) & 1);
		newBits = SetBit(newBits, WINBOND_FAN_DIV_BIT1[i], 
								(divisorBits >> 1) & 1);
		newBits = SetBit(newBits, WINBOND_FAN_DIV_BIT0[i], 
								divisorBits & 1);
	}
	
	// write new fan divisors 
	for (int i = 4; i >= 0; i--) 
	{
		UInt8 oldByte = (UInt8)(bits & 0xFF);
		UInt8 newByte = (UInt8)(newBits & 0xFF);
		bits = bits >> 8;
		newBits = newBits >> 8;
		if (oldByte != newByte) 
			WriteByte(0, WINBOND_FAN_BIT_REG[i], newByte);        
	}
}

bool Winbond::Probe()
{	
	Model = UnknownModel;
	
	for (int i = 0; i < 2; i++) 
	{
		SetPorts(i);
		
		Enter();
		
		UInt8 id = SuperIO::ReadByte(SUPERIO_CHIP_ID_REGISTER);
		UInt8 revision = SuperIO::ReadByte(SUPERIO_CHIP_REVISION_REGISTER);
		
		switch (id) 
		{		
			case 0x52:
			{
				switch (revision)
				{
					case 0x17:
					case 0x3A:
					case 0x41:
						Model = W83627HF;
						break;
				}
				break;
			}
				
			case 0x82:
			{
				switch (revision)
				{
					case 0x83:
						Model = W83627THF;
						break;
				}
				break;
			}
				
			case 0x85:
			{
				switch (revision)
				{
					case 0x41:
						Model = W83687THF;
						break;
				}
				break;
			}
				
			case 0x88:
			{
				switch (revision & 0xF0)
				{
					case 0x50:
					case 0x60:
						Model = W83627EHF;
						break;
				}
				break;
			}
				
			case 0xA0:
			{
				switch (revision & 0xF0)
				{
					case 0x20: 
						Model = W83627DHG; 
						break;   
				}
				break;
			}
				
			case 0xA5:
			{
				switch (revision & 0xF0)
				{
					case 0x10:
						Model = W83667HG;
						break;
				}
				break;
			}
				
			case 0xB0:
			{
				switch (revision & 0xF0)
				{
					case 0x70:
						Model = W83627DHGP;
						break;
				}
				break;
			}
				
			case 0xB3:
			{
				switch (revision & 0xF0)
				{
					case 0x50:
						Model = W83667HGB;
						break;
				}
				break; 
			}
		}
		
		if (Model == UnknownModel)
		{
			Exit();
			continue;
		} 
		else
		{
			Select(WINBOND_HARDWARE_MONITOR_LDN);
			
			Address = ReadWord(SUPERIO_BASE_ADDRESS_REGISTER);          
			
			//usleep(1);
			
			UInt16 verify = ReadWord(SUPERIO_BASE_ADDRESS_REGISTER);
			
			Exit();
			
			if (Address != verify || Address < 0x100 || (Address & 0xF007) != 0)
				continue;
						
			return true;
		}
	}
	
	return false;
}

void Winbond::Init()
{	
	char value[2];
	
	WinbondInstance = this;
	
	switch (Model) 
	{
		case W83667HG:
		case W83667HGB:
		{
			// do not add temperature sensor registers that read PECI
			UInt8 flag = ReadByte(0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
			
			// Heatsink
			if ((flag & 0x04) == 0)	FakeSMCAddKeyCallback("Th0H", "sp78", 2, value, &::Update);
			
			/*if ((flag & 0x40) == 0)
			 list.Add(new Sensor(TEMPERATURE_NAME[1], 1, null,
			 SensorType.Temperature, this, parameter));*/
			
			// Northbridge
			FakeSMCAddKeyCallback("TN0P", "sp78", 2, value, &::Update);
			
			break;
		}
			
		case W83627DHG:        
		case W83627DHGP:
		{
			// do not add temperature sensor registers that read PECI
			UInt8 sel = ReadByte(0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
			
			// Heatsink
			if ((sel & 0x07) == 0) FakeSMCAddKeyCallback("Th0H", "sp78", 2, value, &::Update);
			
			/*if ((sel & 0x70) == 0)
			 list.Add(new Sensor(TEMPERATURE_NAME[1], 1, null,
			 SensorType.Temperature, this, parameter));*/
			
			// Northbridge
			FakeSMCAddKeyCallback("TN0P", "sp78", 2, value, &::Update);
			
			break;
		}
			
		default:
		{
			// no PECI support, add all sensors
			// Heatsink
			FakeSMCAddKeyCallback("Th0H", "sp78", 2, value, &::Update);
			// Northbridge
			FakeSMCAddKeyCallback("TN0P", "sp78", 2, value, &::Update);
			break;
		}
	}
	
	// CPU Vcore
	FakeSMCAddKeyCallback("VC0C", "fp2e", 2, value, &::Update);
	FakeSMCAddKeyCallback("VC0c", "ui16", 2, value, &::Update);
	
	// FANs
	FanOffset = GetFNum();
	
	UpdateRPM();
	
	for (int i = 0; i < 5; i++) 
	{
		char key[5];
		
		if (/*(fanName[i] != NULL && fanName[i]->getLength() > 0) ||*/ FanValue[i] > 0)
		{	
			/*snprintf(key, 5, "F%dID", FanOffset + FanCount);
			FakeSMCAddKey(key, "ch8*", fanName[i]->getLength(), (char*)fanName[i]->getCStringNoCopy());*/
			
			snprintf(key, 5, "F%dAc", FanOffset + FanCount);
			FakeSMCAddKeyCallback(key, "fpe2", 2, value, &::Update);
			
			FanIndex[FanCount++] = i;
		}
	}
	
	UpdateFNum(FanCount);
}

void Winbond::Finish()
{
	FakeSMCRemoveKeyCallback("Th0H");
	FakeSMCRemoveKeyCallback("TN0P");
	FakeSMCRemoveKeyCallback("VC0C");
	FakeSMCRemoveKeyCallback("VC0c");
	for (int i = FanOffset; i < FanOffset + FanCount; i++) 
	{
		char key[5];
		snprintf(key, 5, "F%dAc", i);
		FakeSMCRemoveKeyCallback(key);
	}
	UpdateFNum(-FanCount);
}

void Winbond::Update(const char *key, char *data)
{
	// Heatsink
	if(CompareKeys(key, "Th0H"))
	{
		data[0] = ReadTemperature(0);
		data[1] = 0;
	}
	
	// Northbridge
	if(CompareKeys(key, "TN0P"))
	{
		data[0] = ReadTemperature(2);
		data[1] = 0;
	}
	
	// CPU Vcore
	if(CompareKeys(key, "VC0C"))
	{
		float voltage; 
		
		switch (Model) 
		{
			case W83627HF:
			case W83627THF:
			case W83687THF:
			{
				UInt8 vrmConfiguration = ReadByte(0, 0x18);
				LastVcore = ReadByte(0, WINBOND_VOLTAGE_BASE_REG);
				
				if ((vrmConfiguration & 0x01) == 0)
					voltage = 0.016f * LastVcore; // VRM8 formula
				else
					voltage = 0.00488f * LastVcore + 0.69f; // VRM9 formula
				
				break;
			}
			default:
			{
				LastVcore = ReadByte(0, WINBOND_VOLTAGE_BASE_REG);
				voltage = 0.008f * LastVcore;
				
				break;
			}
		}
		
		UInt16 base = voltage * 1000;
		UInt16 dec = base / 1000;
		UInt16 frc = base - (dec * 1000);
		
		UInt16 value = (dec << 14) | (frc << 4);
		
		data[0] = (value & 0xff00) >> 8;
		data[1] = (value & 0x00ff) | 0x3;
	}
	
	if(CompareKeys(key, "VC0c"))
	{
		data[0] = (LastVcore & 0xff00) >> 8;
		data[1] = LastVcore & 0x00ff;
	}
	
	// Fans
	for (int i = FanOffset; i < FanOffset + FanCount; i++)
	{
		char name[5];
		
		snprintf(name, 5, "F%dAc", i);
		
		if(CompareKeys(key, name))
		{
			UInt8 index = FanIndex[key[1] - 48 - FanOffset];
			
			if(FanValueObsolete[index])
				UpdateRPM();
			
			UInt16 value = FanValue[index];
			
			FanValueObsolete[index] = true;
			
			data[0] = (value >> 6) & 0xff;
			data[1] = (value << 2) & 0xff;
		}
	}
}