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
#include "WinbondFanController.h"
#include "cpuid.h"

UInt8 Winbond::ReadByte(UInt8 bank, UInt8 reg) 
{
	outb((UInt16)(m_Address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(m_Address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(m_Address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	return inb((UInt16)(m_Address + WINBOND_DATA_REGISTER_OFFSET));
}

void Winbond::WriteByte(UInt8 bank, UInt8 reg, UInt8 value)
{
	outb((UInt16)(m_Address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(m_Address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(m_Address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	outb((UInt16)(m_Address + WINBOND_DATA_REGISTER_OFFSET), value); 
}

SInt16 Winbond::ReadTemperature(UInt8 index)
{
	UInt32 value = ReadByte(WINBOND_TEMPERATURE_BANK[index], WINBOND_TEMPERATURE[index]) << 1;
	
	if (WINBOND_TEMPERATURE_BANK[index] > 0) 
		value |= ReadByte(WINBOND_TEMPERATURE_BANK[index], (UInt8)(WINBOND_TEMPERATURE[index] + 1)) >> 7;
	
	float temperature = (float)value / 2.0f;
	
	return temperature <= 125 && temperature >= -55 ? temperature : 0;
}

SInt16 Winbond::ReadVoltage(UInt8 index)
{
	float voltage = 0;
	float gain = 1;
	
	UInt16 V = ReadByte(0, WINBOND_VOLTAGE + index);
	
	if (index = 0) 
		m_RawVCore = V;
	
	if (index == 0 && (m_Model == W83627HF || m_Model == W83627THF || m_Model == W83687THF)) 
	{
		UInt8 vrmConfiguration = ReadByte(0, 0x18);
		
		if ((vrmConfiguration & 0x01) == 0)
			voltage = 16.0f * V; // VRM8 formula
		else
			voltage = 4.88f * V + 690.0f; // VRM9 formula
	}
	else 
	{
		if (index == 3) gain = 2;
		
		voltage = (V << 3) * gain;
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
		bool newAlgo = false;
		
		if (newAlgo) 
		{
			for (int i = 0; i < m_FanLimit; i++)
			{
				UInt8 reg0 = ReadByte(0, WINBOND_TACHOMETER_DIV0[i]);
				UInt8 reg1 = ReadByte(0, WINBOND_TACHOMETER_DIV1[i]);
				UInt8 reg2 = ReadByte(0, WINBOND_TACHOMETER_DIV2[i]);
				
				UInt8 offset =	(((reg2 >> WINBOND_TACHOMETER_DIV2_BIT[i]) & 0x01) << 2) |
				(((reg1 >> WINBOND_TACHOMETER_DIV1_BIT[i]) & 0x01) << 1) |
				((reg0 >> WINBOND_TACHOMETER_DIV0_BIT[i]) & 0x01);
				
				UInt8 divisor = 1 << offset;
				
				UInt8 count = ReadByte(WINBOND_TACHOMETER_BANK[i], WINBOND_TACHOMETER[i]);
				
				m_FanValue[i] = (count < 0xff) ? 1.35e6f / (float)(count * divisor) : 0;
				m_FanValueObsolete[i] = false;
				
				UInt8 oldOffset = offset;
				
				if (count > 192 && offset < 7) 
					offset++;
				if (count < 96 && offset > 0)
					offset--;
				
				// Update divisors
				if (offset != oldOffset)
				{
					WriteByte(0, WINBOND_TACHOMETER_DIV0[i], reg0 | (( offset       & 0x01) << WINBOND_TACHOMETER_DIV0_BIT[i]));
					WriteByte(0, WINBOND_TACHOMETER_DIV1[i], reg1 | (((offset >> 1) & 0x01) << WINBOND_TACHOMETER_DIV1_BIT[i]));
					WriteByte(0, WINBOND_TACHOMETER_DIV2[i], reg2 | (((offset >> 2) & 0x01) << WINBOND_TACHOMETER_DIV2_BIT[i]));
				}
			}
		}
		else 
		{
			UInt64 bits = 0;
			
			for (int i = 0; i < m_FanLimit; i++)
				bits = (bits << 8) | ReadByte(0, WINBOND_TACHOMETER_DIVISOR[i]);
			
			bits = bits << ((5 - m_FanLimit) * 8);
		
			UInt64 newBits = bits;
				
			for (int i = 0; i < m_FanLimit; i++)
			{
				// assemble fan divisor
				UInt8 divisorBits = (int)(
										(((bits >> WINBOND_TACHOMETER_DIVISOR2[i]) & 1) << 2) |
										(((bits >> WINBOND_TACHOMETER_DIVISOR1[i]) & 1) << 1) |
										 ((bits >> WINBOND_TACHOMETER_DIVISOR0[i]) & 1));
				
				UInt8 divisor = 1 << divisorBits;
				
				UInt8 count = ReadByte(WINBOND_TACHOMETER_BANK[i], WINBOND_TACHOMETER[i]);
				
				m_FanValue[i] = (count < 0xff) ? 1.35e6f / (float(count * divisor)) : 0;
				m_FanValueObsolete[i] = false;
				
				// update fan divisor
				if (count > 192 && divisorBits < 7) 
					divisorBits++;
				if (count < 96 && divisorBits > 0)
					divisorBits--;
				
				newBits = SetBit(newBits, WINBOND_TACHOMETER_DIVISOR2[i], (divisorBits >> 2) & 1);
				newBits = SetBit(newBits, WINBOND_TACHOMETER_DIVISOR1[i], (divisorBits >> 1) & 1);
				newBits = SetBit(newBits, WINBOND_TACHOMETER_DIVISOR0[i],  divisorBits       & 1);
			}		
			
			bits = bits >> ((5 - m_FanLimit) * 8);
			newBits = newBits >> ((5 - m_FanLimit) * 8);
			
			// write new fan divisors 
			for (int i = m_FanLimit - 1; i >= 0; i--) 
			{
				UInt8 oldByte = (UInt8)(bits & 0xFF);
				UInt8 newByte = (UInt8)(newBits & 0xFF);
				bits = bits >> 8;
				newBits = newBits >> 8;
				if (oldByte != newByte) 
					WriteByte(0, WINBOND_TACHOMETER_DIVISOR[i], newByte);        
			}
		}
	}
	
	m_FanValueObsolete[index] = true;
	
	return m_FanValue[index];
}

void Winbond::Enter()
{
	outb(m_RegisterPort, 0x87);
	outb(m_RegisterPort, 0x87);
}

void Winbond::Exit()
{
	outb(m_RegisterPort, 0xAA);
	outb(m_RegisterPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	outb(m_ValuePort, 0x02);
}

bool Winbond::ProbePort()
{	
	UInt8 id = ListenPortByte(SUPERIO_CHIP_ID_REGISTER);
	UInt8 revision = ListenPortByte(SUPERIO_CHIP_REVISION_REGISTER);
	
	if (id == 0 || id == 0xff || revision == 0 || revision == 0xff)
		return false;
	
	m_FanLimit = 3;
	
	switch (id) 
	{		
		case 0x52:
		{
			switch (revision & 0xf0)
			{
				case 0x10:
				case 0x30:
				case 0x40:
					m_Model = W83627HF;
					break;
				/*case 0x70:
					m_Model = W83977CTF;
					break;
				case 0xf0:
					m_Model = W83977EF;
					break;*/
					
			}
		}
		case 0x59:
		{
			switch (revision & 0xf0)
			{
				case 0x50:
					m_Model = W83627SF;
					break;						
			}
			break;
		}
			
		case 0x60:
		{
			switch (revision & 0xf0)
			{
				case 0x10:
					m_Model = W83697HF;
					m_FanLimit = 2;
					break;						
			}
			break;
		}
			
		/*case 0x61:
		{
			switch (revision & 0xf0)
			{
				case 0x00:
					m_Model = W83L517D;
					break;						
			}
			break;
		}*/
			
		case 0x68:
		{
			switch (revision & 0xf0)
			{
				case 0x10:
					m_Model = W83697SF;
					m_FanLimit = 2;
					break;						
			}
			break;
		}
			
		case 0x70:
		{
			switch (revision & 0xf0)
			{
				case 0x80:
					m_Model = W83637HF;
					m_FanLimit = 5;
					break;						
			}
			break;
		}
			
			
		case 0x82:
		{
			switch (revision)
			{
				case 0x83:
					m_Model = W83627THF;
					break;
			}
			break;
		}
			
		case 0x85:
		{
			switch (revision)
			{
				case 0x41:
					m_Model = W83687THF;
					// No datasheet
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
					m_Model = W83627EHF;
					m_FanLimit = 5;
					break;
			}
			break;
		}
			
		/*case 0x97:
		{
			switch (revision)
			{
				case 0x71:
					m_Model = W83977FA;
					break;
				case 0x73:
					m_Model = W83977TF;
					break;
				case 0x74:
					m_Model = W83977ATF;
					break;
				case 0x77:
					m_Model = W83977AF;
					break;
			}
			break;
		}*/	
			
		case 0xA0:
		{
			switch (revision & 0xF0)
			{
				case 0x20: 
					m_Model = W83627DHG;
					m_FanLimit = 5;
					break;   
			}
			break;
		}
			
		case 0xA2:
		{
			switch (revision & 0xF0)
			{
				case 0x30: 
					m_Model = W83627UHG; 
					m_FanLimit = 2;
					break;   
			}
			break;
		}
			
		case 0xA5:
		{
			switch (revision & 0xF0)
			{
				case 0x10:
					m_Model = W83667HG;
					m_FanLimit = 2;
					break;
			}
			break;
		}
			
		case 0xB0:
		{
			switch (revision & 0xF0)
			{
				case 0x70:
					m_Model = W83627DHGP;
					m_FanLimit = 5;
					break;
			}
			break;
		}
			
		case 0xB3:
		{
			switch (revision & 0xF0)
			{
				case 0x50:
					m_Model = W83667HGB;
					break;
			}
			break; 
		}
			
		/*default: 
		{
			switch (id & 0x0f) {
				case 0x0a:
					m_Model = W83877F;
					break;
				case 0x0b:
					m_Model = W83877AF;
					break;
				case 0x0c:
					m_Model = W83877TF;
					break;
				case 0x0d:
					m_Model = W83877ATF;
					break;
			}
		}*/
	}
	
	if (m_Model == UnknownModel)
	{
		InfoLog("Found unsupported chip ID=0x%x REVISION=0x%x", id, revision);
		return false;
	}
	
	Select(WINBOND_HARDWARE_MONITOR_LDN);
	
	m_Address = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);          
	
	IOSleep(1000);
	
	UInt16 verify = ListenPortWord(SUPERIO_BASE_ADDRESS_REGISTER);

	if (m_Address != verify || m_Address < 0x100 || (m_Address & 0xF007) != 0)
		return false;
	
	return true;
}

void Winbond::Start()
{	
	bool isCpuCore_i = false;
		
	if (strcmp(cpuid_info()->cpuid_vendor, CPUID_VID_INTEL) != 0) 
	{
		switch (cpuid_info()->cpuid_family)
		{
			case 0x6:
			{
				switch (cpuid_info()->cpuid_model)
				{
					case 0x1A: // Intel Core i7 LGA1366 (45nm)
					case 0x1E: // Intel Core i5, i7 LGA1156 (45nm)
					case 0x25: // Intel Core i3, i5, i7 LGA1156 (32nm)
					case 0x2C: // Intel Core i7 LGA1366 (32nm) 6 Core
						isCpuCore_i = true;
						break;
				}
			}	break;
		}
	}
	
	if (isCpuCore_i)
	{
		// Heatsink
		AddSensor(new WinbondTemperatureSensor(this, 2, KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2));
	}
	else 
	{	
		switch (m_Model) 
		{
			case W83667HG:
			case W83667HGB:
			{
				// do not add temperature sensor registers that read PECI
				UInt8 flag = ReadByte(0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
				
				if ((flag & 0x04) == 0)	
				{
					// Heatsink
					AddSensor(new WinbondTemperatureSensor(this, 0, KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2));
				}
				//else if ((flag & 0x40) == 0)
				//{
				//	// Heatsink
				//	AddSensor(new WinbondTemperatureSensor(this, 1, KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2));
				//}

				// Northbridge
				AddSensor(new WinbondTemperatureSensor(this, 2, KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2));
				
				break;
			}
				
			case W83627DHG:        
			case W83627DHGP:
			{
				// do not add temperature sensor registers that read PECI
				UInt8 sel = ReadByte(0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
				
				if ((sel & 0x07) == 0) 
				{
					// Heatsink
					AddSensor(new WinbondTemperatureSensor(this, 0, KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2));
				}
				//else if ((sel & 0x70) == 0)
				//{
				//	// Heatsink
				//	AddSensor(new WinbondTemperatureSensor(this, 1, KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2));
				//}
				
				// Northbridge
				AddSensor(new WinbondTemperatureSensor(this, 2, KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2));
				
				break;
			}
				
			default:
			{
				// no PECI support, add all sensors
				// Heatsink
				AddSensor(new WinbondTemperatureSensor(this, 0, KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2));
				// Northbridge
				AddSensor(new WinbondTemperatureSensor(this, 2, KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2));
				break;
			}
		}
	}
	
	// CPU Vcore
	AddSensor(new WinbondVoltageSensor(this, 0, KEY_CPU_VOLTAGE, TYPE_FP2E, 2));
	
	// FANs
	ReadTachometer(0, true);
	
	for (int i = 0; i < m_FanLimit; i++) 
	{
		char* key = (char*)IOMalloc(5);
		
		if (m_FanForced[i] || ReadTachometer(i, false) > 0)
		{
			int offset = GetNextFanIndex();
			
			if (offset != -1) 
			{
				if (m_FanName[i] && strlen(m_FanName[i]) > 0)
				{
					snprintf(key, 5, KEY_FORMAT_FAN_ID, offset); 
					FakeSMCAddKey(key, TYPE_CH8, strlen(m_FanName[i]), (char*)m_FanName[i]);
					
					InfoLog("%s name is associated with hardware Fan%d", m_FanName[i], i);
				}
				else 
				{
					InfoLog("Fan %d name is associated with hardware Fan%d", offset, i);
				}
				
				snprintf(key, 5, KEY_FORMAT_FAN_SPEED, offset); 
				AddSensor(new WinbondTachometerSensor(this, i, key, TYPE_FPE2, 2));
				
				m_FanIndex[m_FanCount++] = i;
			}
			
			// Fan Control Support
			if (m_FanControlEnabled && m_FanControl[i])
				AddController(new WinbondFanController(this, i));
		}
		
		IOFree(key, 5);
	}
	
	UpdateFNum();
}
