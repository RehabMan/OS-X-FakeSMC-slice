/*
 *  W836x.cpp
 *  HWSensors
 *
 *  Created by mozo on 14/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "W836x.h"
#include "FakeSMC.h"
#include "cpuid.h"
#include <architecture/i386/pio.h>

#define Debug FALSE

#define LogPrefix "W836x: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super SuperIOMonitor
OSDefineMetaClassAndStructors(W836x, SuperIOMonitor)

UInt8 W836x::readByte(UInt8 bank, UInt8 reg) 
{
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	return inb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET));
}

void W836x::writeByte(UInt8 bank, UInt8 reg, UInt8 value)
{
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), WINBOND_BANK_SELECT_REGISTER);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), bank);
	outb((UInt16)(address + WINBOND_ADDRESS_REGISTER_OFFSET), reg);
	outb((UInt16)(address + WINBOND_DATA_REGISTER_OFFSET), value); 
}

long W836x::readTemperature(unsigned long index)
{
	UInt32 value = readByte(WINBOND_TEMPERATURE_BANK[index], WINBOND_TEMPERATURE[index]) << 1;
	
	if (WINBOND_TEMPERATURE_BANK[index] > 0) 
		value |= readByte(WINBOND_TEMPERATURE_BANK[index], (UInt8)(WINBOND_TEMPERATURE[index] + 1)) >> 7;
	
	float temperature = (float)value / 2.0f;
	
	return temperature <= 125 && temperature >= -55 ? temperature : 0;
}

long W836x::readVoltage(unsigned long index)
{
	float voltage = 0;
	float gain = 1;
	
	UInt16 V = readByte(0, WINBOND_VOLTAGE + index);
	
	if (index == 0 && (model == W83627HF || model == W83627THF || model == W83687THF)) 
	{
		UInt8 vrmConfiguration = readByte(0, 0x18);
		
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

UInt64 set_bit(UInt64 target, UInt32 bit, UInt32 value)
{
	if (((value & 1) == value) && bit >= 0 && bit <= 63)
	{
		UInt64 mask = (((UInt64)1) << bit);
		return value > 0 ? target | mask : target & ~mask;
	}
	
	return value;
}

void W836x::updateTachometers()
{
	UInt64 bits = 0;
	
	for (int i = 0; i < 5; i++)
	{
		bits = (bits << 8) | readByte(0, WINBOND_TACHOMETER_DIVISOR[i]);
	}
	
	UInt64 newBits = bits;
	
	for (int i = 0; i < fanLimit; i++)
	{
		// assemble fan divisor
		UInt8 offset =	(((bits >> WINBOND_TACHOMETER_DIVISOR2[i]) & 1) << 2) |
		(((bits >> WINBOND_TACHOMETER_DIVISOR1[i]) & 1) << 1) |
		((bits >> WINBOND_TACHOMETER_DIVISOR0[i]) & 1);
		
		UInt8 divisor = 1 << offset;
		UInt8 count = readByte(WINBOND_TACHOMETER_BANK[i], WINBOND_TACHOMETER[i]);
		
		// update fan divisor
		if (count > 192 && offset < 7)
		{
			offset++;
		}
		else if (count < 96 && offset > 0)
		{
			offset--;
		}
		
		fanValue[i] = (count < 0xff) ? 1.35e6f / (float(count * divisor)) : 0;
		fanValueObsolete[i] = false;
		
		newBits = set_bit(newBits, WINBOND_TACHOMETER_DIVISOR2[i], (offset >> 2) & 1);
		newBits = set_bit(newBits, WINBOND_TACHOMETER_DIVISOR1[i], (offset >> 1) & 1);
		newBits = set_bit(newBits, WINBOND_TACHOMETER_DIVISOR0[i],  offset       & 1);
	}		
	
	// write new fan divisors 
	for (int i = 4; i >= 0; i--) 
	{
		UInt8 oldByte = bits & 0xff;
		UInt8 newByte = newBits & 0xff;
		
		if (oldByte != newByte)
		{
			writeByte(0, WINBOND_TACHOMETER_DIVISOR[i], newByte);
		}
		
		bits = bits >> 8;
		newBits = newBits >> 8;
	}
}


long W836x::readTachometer(unsigned long index)
{
	if (fanValueObsolete[index])
		updateTachometers();
	
	fanValueObsolete[index] = true;
	
	return fanValue[index];
}

void W836x::enter()
{
	outb(registerPort, 0x87);
	outb(registerPort, 0x87);
}

void W836x::exit()
{
	outb(registerPort, 0xAA);
	outb(registerPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	outb(valuePort, 0x02);
}

bool W836x::probePort()
{
	UInt8 id =listenPortByte(SUPERIO_CHIP_ID_REGISTER);
	UInt8 revision = listenPortByte(SUPERIO_CHIP_REVISION_REGISTER);
	
	if (id == 0 || id == 0xff || revision == 0 || revision == 0xff)
		return false;
	
	fanLimit = 3;
	
	switch (id) 
	{		
		case 0x52:
		{
			switch (revision & 0xf0)
			{
				case 0x10:
				case 0x30:
				case 0x40:
				case 0x41:
					model = W83627HF;
					break;
					/*case 0x70:
					 model = W83977CTF;
					 break;
					 case 0xf0:
					 model = W83977EF;
					 break;*/
					
			}
		}
		case 0x59:
		{
			switch (revision & 0xf0)
			{
				case 0x50:
					model = W83627SF;
					break;						
			}
			break;
		}
			
		case 0x60:
		{
			switch (revision & 0xf0)
			{
				case 0x10:
					model = W83697HF;
					fanLimit = 2;
					break;						
			}
			break;
		}
			
			/*case 0x61:
			 {
			 switch (revision & 0xf0)
			 {
			 case 0x00:
			 model = W83L517D;
			 break;						
			 }
			 break;
			 }*/
			
		case 0x68:
		{
			switch (revision & 0xf0)
			{
				case 0x10:
					model = W83697SF;
					fanLimit = 2;
					break;						
			}
			break;
		}
			
		case 0x70:
		{
			switch (revision & 0xf0)
			{
				case 0x80:
					model = W83637HF;
					fanLimit = 5;
					break;						
			}
			break;
		}
			
			
		case 0x82:
		{
			switch (revision & 0xF0)
			{
				case 0x80:
					model = W83627THF;
					break;
			}
			break;
		}
			
		case 0x85:
		{
			switch (revision)
			{
				case 0x41:
					model = W83687THF;
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
					model = W83627EHF;
					fanLimit = 5;
					break;
			}
			break;
		}
			
			/*case 0x97:
			 {
			 switch (revision)
			 {
			 case 0x71:
			 model = W83977FA;
			 break;
			 case 0x73:
			 model = W83977TF;
			 break;
			 case 0x74:
			 model = W83977ATF;
			 break;
			 case 0x77:
			 model = W83977AF;
			 break;
			 }
			 break;
			 }*/	
			
		case 0xA0:
		{
			switch (revision & 0xF0)
			{
				case 0x20: 
					model = W83627DHG;
					fanLimit = 5;
					break;   
			}
			break;
		}
			
		case 0xA2:
		{
			switch (revision & 0xF0)
			{
				case 0x30: 
					model = W83627UHG; 
					fanLimit = 2;
					break;   
			}
			break;
		}
			
		case 0xA5:
		{
			switch (revision & 0xF0)
			{
				case 0x10:
					model = W83667HG;
					fanLimit = 2;
					break;
			}
			break;
		}
			
		case 0xB0:
		{
			switch (revision & 0xF0)
			{
				case 0x70:
					model = W83627DHGP;
					fanLimit = 5;
					break;
			}
			break;
		}
			
		case 0xB3:
		{
			switch (revision & 0xF0)
			{
				case 0x50:
					model = W83667HGB;
					break;
			}
			break; 
		}
			
			/*default: 
			 {
			 switch (id & 0x0f) {
			 case 0x0a:
			 model = W83877F;
			 break;
			 case 0x0b:
			 model = W83877AF;
			 break;
			 case 0x0c:
			 model = W83877TF;
			 break;
			 case 0x0d:
			 model = W83877ATF;
			 break;
			 }
			 }*/
	}
	
	if (!model)
	{
		InfoLog("found unsupported chip ID=0x%x REVISION=0x%x", id, revision);
		return false;
	}
	
	selectLogicalDevice(WINBOND_HARDWARE_MONITOR_LDN);
	
	if (!getLogicalDeviceAddress())
		return false;
	
	return true;
}

const char *W836x::getModelName()
{
	switch (model) 
	{
        case W83627DHG: return "W83627DHG";
        case W83627DHGP: return "W83627DHG-P";
        case W83627EHF: return "W83627EHF";
        case W83627HF: return "W83627HF";
        case W83627THF: return "W83627THF";
        case W83667HG: return "W83667HG";
        case W83667HGB: return "W83667HG-B";
        case W83687THF: return "W83687THF";
		case W83627SF: return "W83627SF";
        case W83697HF: return "W83697HF";
		case W83637HF: return "W83637HF";
        case W83627UHG: return "W83627UHG";
        case W83697SF: return "W83697SF";
	}
	
	return "unknown";
}

bool W836x::init(OSDictionary *properties)
{
	DebugLog("initialising...");
	
    if (!super::init(properties))
		return false;
	
	return true;
}

IOService* W836x::probe(IOService *provider, SInt32 *score)
{
	DebugLog("probing...");
	
	if (super::probe(provider, score) != this) 
		return 0;
	
	return this;
}

bool W836x::start(IOService * provider)
{
	DebugLog("starting...");
	
	if (!super::start(provider)) 
		return false;
	
	InfoLog("found Winbond %s", getModelName());
	
	OSDictionary* configuration = OSDynamicCast(OSDictionary, getProperty("Sensors Configuration"));
	
	OSBoolean* tempin0forced = configuration ? OSDynamicCast(OSBoolean, configuration->getObject("TEMPIN0FORCED")) : 0;
	OSBoolean* tempin1forced = configuration ? OSDynamicCast(OSBoolean, configuration->getObject("TEMPIN1FORCED")) : 0;
	
	if (OSNumber* fanlimit = configuration ? OSDynamicCast(OSNumber, configuration->getObject("FANINLIMIT")) : 0)
		fanLimit = fanlimit->unsigned8BitValue();
	
	cpuid_update_generic_info();
	
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
		if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 2))
			return false;
	}
	else 
	{	
		switch (model) 
		{
			case W83667HG:
			case W83667HGB:
			{
				// do not add temperature sensor registers that read PECI
				UInt8 flag = readByte(0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
				
				if ((flag & 0x04) == 0 || (tempin0forced && tempin0forced->getValue()))
				{
					// Heatsink
					if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 0))
						WarningLog("error adding heatsink temperature sensor");
				}
				else if ((flag & 0x40) == 0 || (tempin1forced && tempin1forced->getValue()))
				{
					// Ambient
					if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 1))
						WarningLog("error adding ambient temperature sensor");
				}
				
				// Northbridge
				if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 2))
					WarningLog("error adding system temperature sensor");
				
				break;
			}
				
			case W83627DHG:        
			case W83627DHGP:
			{
				// do not add temperature sensor registers that read PECI
				UInt8 sel = readByte(0, WINBOND_TEMPERATURE_SOURCE_SELECT_REG);
				
				if ((sel & 0x07) == 0 || (tempin0forced && tempin0forced->getValue())) 
				{
					// Heatsink
					if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 0))
						WarningLog("error adding heatsink temperature sensor");
				}
				else if ((sel & 0x70) == 0 || (tempin1forced && tempin1forced->getValue()))
				{
					// Ambient
					if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 1))
						WarningLog("error adding ambient temperature sensor");
				}
				
				// Northbridge
				if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 2))
					WarningLog("error adding system temperature sensor");
				
				break;
			}
				
			default:
			{
				// no PECI support, add all sensors
				
				// Heatsink
				if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 0))
					WarningLog("error adding heatsink temperature sensor");
				// Ambient

				if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 1))
					WarningLog("error adding ambient temperature sensor");

				// Northbridge
				if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 2))
					WarningLog("error adding system temperature sensor");
				
				break;
			}
		}
	}
	
	// CPU Vcore
	if (!addSensor(KEY_CPU_VOLTAGE, TYPE_FP2E, 2, kSuperIOVoltageSensor, 0))
		WarningLog("error adding CPU voltage sensor");
	
	// FANs
	for (int i = 0; i < fanLimit; i++) 
		fanValueObsolete[i] = true;
	
	updateTachometers();
	
	for (int i = 0; i < fanLimit; i++) {
		OSString* name = 0;
		
		if (configuration) {
			char key[7];
			
			snprintf(key, 7, "FANIN%X", i);
			
			name = OSDynamicCast(OSString, configuration->getObject(key));
		}
		
		UInt32 nameLength = name ? strlen(name->getCStringNoCopy()) : 0;
		
		if (readTachometer(i) > 10 || nameLength > 0)
			if (!addTachometer(i, (nameLength > 0 ? name->getCStringNoCopy() : 0)))
				WarningLog("error adding tachometer sensor %d", i);
	}
	
	return true;
}

void W836x::stop (IOService* provider)
{
	DebugLog("stoping...");
	
	super::stop(provider);
}

void W836x::free ()
{
	DebugLog("freeing...");
	
	super::free();
}