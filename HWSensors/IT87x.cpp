/*
 *  IT87x.cpp
 *  HWSensors
 *
 *  Created by mozo on 08/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include <architecture/i386/pio.h>
#include "IT87x.h"
#include "FakeSMC.h"

#define Debug FALSE

#define LogPrefix "IT87x: "
#define DebugLog(string, args...)	do { if (Debug) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define super SuperIOMonitor
OSDefineMetaClassAndStructors(IT87x, SuperIOMonitor)

UInt8 IT87x::readByte(UInt8 reg, bool* valid)
{
	outb(address + ITE_ADDRESS_REGISTER_OFFSET, reg);
	
	UInt8 value = inb(address + ITE_DATA_REGISTER_OFFSET);
	
	valid = (bool*)(reg == inb(address + ITE_DATA_REGISTER_OFFSET));
	
	return value;
}

UInt16 IT87x::readWord(UInt8 reg1, UInt8 reg2, bool* valid)
{	
	return readByte(reg1, valid) << 8 | readByte(reg2, valid);
}

void IT87x::writeByte(UInt8 reg, UInt8 value)
{
	outb(address + ITE_ADDRESS_REGISTER_OFFSET, reg);
	outb(address + ITE_DATA_REGISTER_OFFSET, value);
}

long IT87x::readTemperature(unsigned long index)
{
	bool* valid;
	return readByte(ITE_TEMPERATURE_BASE_REG + index, valid);
}

long IT87x::readVoltage(unsigned long index)
{
	bool* valid;
	//Zorglub
	if (model == IT8721F) {
        int gain = 12;
        if ((index == 3) || (index == 7) || (index == 8))
			gain = 24;
        return readByte(ITE_VOLTAGE_BASE_REG + index, valid) * gain;
    }
	return readByte(ITE_VOLTAGE_BASE_REG + index, valid) << 4;
}

long IT87x::readTachometer(unsigned long index)
{
	bool* valid;
	int value = readByte(ITE_FAN_TACHOMETER_REG[index], valid);
	
	value |= readByte(ITE_FAN_TACHOMETER_EXT_REG[index], valid) << 8;
	
	return value > 0x3f && value < 0xffff ? (float)(1350000 + value) / (float)(value * 2) : 0;
}

bool IT87x::probePort()
{	
	UInt16 id = listenPortWord(SUPERIO_CHIP_ID_REGISTER);
	
	if (id == 0 || id == 0xffff)
		return false;
	
	switch (id)
	{
		case IT8512F:
		case IT8712F:
		case IT8716F:
		case IT8718F:
		case IT8720F: 
		case IT8721F: 
		case IT8726F:
		case IT8728F:
		case IT8752F:
			model = id; 
			break; 
		default:
			WarningLog("found unsupported chip ID=0x%x", id);
			return false;
	}
	
	selectLogicalDevice(ITE_ENVIRONMENT_CONTROLLER_LDN);
	
	if (!getLogicalDeviceAddress())
		return false;
	
	bool* valid;
	
	UInt8 vendor = readByte(ITE_VENDOR_ID_REGISTER, valid);
	
	if (!valid || vendor != ITE_VENDOR_ID)
		return false;
	
	if ((readByte(ITE_CONFIGURATION_REGISTER, valid) & 0x10) == 0)
		return false;
	
	if (!valid)
		return false;
	
	return true;
}

void IT87x::enter()
{
	outb(registerPort, 0x87);
	outb(registerPort, 0x01);
	outb(registerPort, 0x55);
	
	if (registerPort == 0x4e) 
	{
		outb(registerPort, 0xaa);
	}
	else
	{
		outb(registerPort, 0x55);
	}
}

void IT87x::exit()
{
	outb(registerPort, SUPERIO_CONFIGURATION_CONTROL_REGISTER);
	outb(valuePort, 0x02);
}

const char *IT87x::getModelName()
{
	switch (model) 
	{
        case IT8512F: return "IT8512F";
        case IT8712F: return "IT8712F";
        case IT8716F: return "IT8716F";
        case IT8718F: return "IT8718F";
        case IT8720F: return "IT8720F";
        case IT8721F: return "IT8721F";
        case IT8726F: return "IT8726F";
		case IT8728F: return "IT8728F";
        case IT8752F: return "IT8752F";
	}
	
	return "unknown";
}

bool IT87x::init(OSDictionary *properties)
{
	DebugLog("initialising...");
	
    if (!super::init(properties))
		return false;
	
	return true;
}

IOService* IT87x::probe(IOService *provider, SInt32 *score)
{
	DebugLog("probing...");
	
	if (super::probe(provider, score) != this) 
		return 0;
		
	return this;
}

bool IT87x::start(IOService * provider)
{
	DebugLog("starting...");
	
	if (!super::start(provider)) 
		return false;
		
	InfoLog("found ITE %s", getModelName());
	
	OSDictionary* configuration = OSDynamicCast(OSDictionary, getProperty("Sensors Configuration"));
	
	// Temperature Sensors
	if (configuration) {
		for (int i = 0; i < 4; i++) 
		{				
			char key[8];
			
			snprintf(key, 8, "TEMPIN%X", i);
			
			if (OSString* name = OSDynamicCast(OSString, configuration->getObject(key)))
				if (name->isEqualTo("Processor")) {
					if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, i))
						WarningLog("error adding heatsink temperature sensor");
				}
				else if (name->isEqualTo("System")) {				
					if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor,i))
						WarningLog("error adding system temperature sensor");
				}
				else if (name->isEqualTo("Auxiliary")) {				
					if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor,i))
						WarningLog("error adding auxiliary temperature sensor");
				}
		}
	}
	else {
		if (!addSensor(KEY_CPU_HEATSINK_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 0))
			WarningLog("error adding heatsink temperature sensor");
		
		if (!addSensor(KEY_AMBIENT_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 1))
			WarningLog("error adding auxiliary temperature sensor");
		
		if (!addSensor(KEY_NORTHBRIDGE_TEMPERATURE, TYPE_SP78, 2, kSuperIOTemperatureSensor, 2))
			WarningLog("error adding system temperature sensor");
	}
	
	
	// Voltage
	if (configuration) {
		for (int i = 0; i < 9; i++) //Zorglub
		{				
			char key[5];
			
			snprintf(key, 5, "VIN%X", i);
			
			if (OSString* name = OSDynamicCast(OSString, configuration->getObject(key))) {
				if (name->isEqualTo("Processor")) {
					if (!addSensor(KEY_CPU_VOLTAGE, TYPE_FP2E, 2, kSuperIOVoltageSensor, i))
						WarningLog("error adding CPU voltage sensor");
				}
				else if (name->isEqualTo("Memory")) {
					if (!addSensor(KEY_MEMORY_VOLTAGE, TYPE_FP2E, 2, kSuperIOVoltageSensor, i))
						WarningLog("error adding memory voltage sensor");
				}
			}
		}
	}
	
	// Tachometers
	for (int i = 0; i < 5; i++) {
		OSString* name = NULL;
		
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

void IT87x::stop (IOService* provider)
{
	DebugLog("stoping...");
		
	super::stop(provider);
}

void IT87x::free ()
{
	DebugLog("freeing...");
	
	super::free();
}