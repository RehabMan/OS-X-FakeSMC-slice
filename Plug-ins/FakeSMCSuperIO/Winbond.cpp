/*
 *  Winbond.cpp
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#include "Winbond.h"
#include "WinbondSensors.h"

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

SInt16 Winbond::ReadTemperature(UInt8 index)
{
	UInt32 value = Winbond::ReadByte(WINBOND_TEMPERATURE_BANK[index], WINBOND_TEMPERATURE_REG[index]) << 1;
	
	if (WINBOND_TEMPERATURE_BANK[index] > 0) 
		value |= Winbond::ReadByte(WINBOND_TEMPERATURE_BANK[index], (UInt8)(WINBOND_TEMPERATURE_REG[index] + 1)) >> 7;
	
	float temperature = (float)value / 2.0f;
	
	return temperature;
}

SInt16 Winbond::ReadVoltage(UInt8 index)
{
	float voltage; 
	
	switch (Model) 
	{
		case W83627HF:
		case W83627THF:
		case W83687THF:
		{
			UInt8 vrmConfiguration = ReadByte(index, 0x18);
			UInt16 V = Winbond::ReadByte(index, WINBOND_VOLTAGE_BASE_REG);
			
			if ((vrmConfiguration & 0x01) == 0)
				voltage = 16.0f * V; // VRM8 formula
			else
				voltage = 4.88f * V + 690.0f; // VRM9 formula
			
			break;
		}
		default:
		{
			UInt16 V = Winbond::ReadByte(index, WINBOND_VOLTAGE_BASE_REG);
			voltage = 8 * V;
			
			break;
		}
	}

	return voltage;
}

UInt64 SetBit(UInt64 target, UInt32 bit, UInt32 value)
{
	if (((value & 1) == value) && bit >= 0 && bit <= 63)
	{
		UInt64 mask = (((UInt64)1) << bit);
		return value > 0 ? target | mask : target & ~mask;
	}
	
	return value;
}

SInt16 Winbond::ReadTachometer(UInt8 index, bool force_update)
{
	if (m_FanValueObsolete[index] || force_update)
	{
		UInt64 bits = 0;
		
		for (int i = 0; i < 5; i++)
			bits = (bits << 8) | Winbond::ReadByte(0, WINBOND_FAN_BIT_REG[i]);
		
		UInt64 newBits = bits;
		
		for (int i = 0; i < 5; i++)
		{
			int count = Winbond::ReadByte(WINBOND_FAN_TACHO_BANK[i], WINBOND_FAN_TACHO_REG[i]);
			
			// assemble fan divisor
			int divisorBits = (int)(
									(((bits >> WINBOND_FAN_DIV_BIT2[i]) & 1) << 2) |
									(((bits >> WINBOND_FAN_DIV_BIT1[i]) & 1) << 1) |
									((bits >> WINBOND_FAN_DIV_BIT0[i]) & 1));
			int divisor = 1 << divisorBits;
			
			m_FanValue[i] = (count < 0xff) ? 1.35e6f / (float)(count * divisor) : 0;
			m_FanValueObsolete[i] = false;
			
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
	
	m_FanValueObsolete[index] = true;
	
	return m_FanValue[index];
}

void Winbond::Enter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x87);
}

void Winbond::Exit()
{
	outb(RegisterPort, 0xAA);
}

bool Winbond::Probe()
{	
	DebugLog("Probing Winbond...");
	
	Model = UnknownModel;
	
	for (int i = 0; i < WINBOND_PORTS_COUNT; i++) 
	{
		RegisterPort	= WINBOND_PORT[i];
		ValuePort		= WINBOND_PORT[i] + 1;
		
		Enter();
		
		UInt8 id = SuperIO::ReadByte(SUPERIO_CHIP_ID_REGISTER);
		UInt8 revision = SuperIO::ReadByte(SUPERIO_CHIP_REVISION_REGISTER);
		
		switch (id) 
		{		
			case 0x52:
			{
				switch (revision & 0xf0)
				{
					case 0x10:
					case 0x30:
					case 0x40:
						Model = W83627HF;
						break;
					case 0x70:
						Model = W83977CTF;
						break;
					case 0xf0:
						Model = W83977EF;
						break;
						
				}
			}
			case 0x59:
			{
				switch (revision & 0xf0)
				{
					case 0x50:
						Model = W83627SF;
						break;						
				}
				break;
			}
				
			case 0x60:
			{
				switch (revision & 0xf0)
				{
					case 0x10:
						Model = W83697HF;
						break;						
				}
				break;
			}
				
			case 0x61:
			{
				switch (revision & 0xf0)
				{
					case 0x00:
						Model = W83L517D;
						break;						
				}
				break;
			}
				
			case 0x68:
			{
				switch (revision & 0xf0)
				{
					case 0x10:
						Model = W83697SF;
						break;						
				}
				break;
			}
				
			case 0x70:
			{
				switch (revision & 0xf0)
				{
					case 0x80:
						Model = W83637HF;
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
				
			case 0x97:
			{
				switch (revision)
				{
					case 0x71:
						Model = W83977FA;
						break;
					case 0x73:
						Model = W83977TF;
						break;
					case 0x74:
						Model = W83977ATF;
						break;
					case 0x77:
						Model = W83977AF;
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
				
			case 0xA2:
			{
				switch (revision & 0xF0)
				{
					case 0x30: 
						Model = W83627UHG; 
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
			default: 
			{
				switch (id & 0x0f) {
					case 0x0a:
						Model = W83877F;
						break;
					case 0x0b:
						Model = W83877AF;
						break;
					case 0x0c:
						Model = W83877TF;
						break;
					case 0x0d:
						Model = W83877ATF;
						break;
					default:
						break;
				}
			}
		}
		
		Select(WINBOND_HARDWARE_MONITOR_LDN);
		
		Address = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);          
		
		IOSleep(1000);
		
		UInt16 verify = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);
		
		Exit();
		
		if (Address != verify || Address < 0x100 || (Address & 0xF007) != 0)
			continue;
		
		if (Model == UnknownModel)
		{
			InfoLog("found unsupported Winbond chip ID=0x%x REVISION=0x%x on ADDRESS=0x%x", id, revision, Address);
			continue;
		} 
		else
		{
			return true;
		}
	}
	
	return false;
}

void Winbond::Init()
{	
	switch (Model) 
	{
		case W83667HG:
		case W83667HGB:
		{
			// do not add temperature sensor registers that read PECI
			UInt8 flag = ReadByte(0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
			
			// Heatsink
			if ((flag & 0x04) == 0)	Bind(new WinbondTemperatureSensor(this, 0, "Th0H", "sp78", 2));
		
			/*if ((flag & 0x40) == 0)
			 list.Add(new Sensor(TEMPERATURE_NAME[1], 1, null,
			 SensorType.Temperature, this, parameter));*/
			
			// Northbridge
			Bind(new WinbondTemperatureSensor(this, 2, "TN0P", "sp78", 2));
			
			break;
		}
			
		case W83627DHG:        
		case W83627DHGP:
		{
			// do not add temperature sensor registers that read PECI
			UInt8 sel = ReadByte(0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
			
			// Heatsink
			if ((sel & 0x07) == 0) Bind(new WinbondTemperatureSensor(this, 0, "Th0H", "sp78", 2));
			
			/*if ((sel & 0x70) == 0)
			 list.Add(new Sensor(TEMPERATURE_NAME[1], 1, null,
			 SensorType.Temperature, this, parameter));*/
			
			// Northbridge
			Bind(new WinbondTemperatureSensor(this, 2, "TN0P", "sp78", 2));
			
			break;
		}
			
		default:
		{
			// no PECI support, add all sensors
			// Heatsink
			Bind(new WinbondTemperatureSensor(this, 0, "Th0H", "sp78", 2));
			// Northbridge
			Bind(new WinbondTemperatureSensor(this, 2, "TN0P", "sp78", 2));
			break;
		}
	}
	
	// CPU Vcore
	Bind(new WinbondVoltageSensor(this, 0, "VC0C", "fp2e", 2));
	//FakeSMCAddKey("VC0c", "ui16", 2, value, this);
	
	// FANs
	FanOffset = GetFNum();
	
	ReadTachometer(0, true);
	
	for (int i = 0; i < 5; i++) 
	{
		char key[5];
		bool fanName = FanName[i] && strlen(FanName[i]) > 0;
		
		if (fanName || m_FanValue[i] > 0)
		{	
			if(fanName)
			{
				snprintf(key, 5, "F%dID", FanOffset + FanCount);
				FakeSMCAddKey(key, "ch8*", strlen(FanName[i]), (char*)FanName[i]);
			}
			
			snprintf(key, 5, "F%dAc", FanOffset + FanCount);
			Bind(new WinbondTachometerSensor(this, i, key, "fpe2", 2));
			
			FanIndex[FanCount++] = i;
		}
	}
	
	UpdateFNum(FanCount);
}

void Winbond::Finish()
{
	FlushBindings();
	UpdateFNum(-FanCount);
}
