/*
 *  SuperIO.h
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#ifndef _SUPERIO_H 
#define _SUPERIO_H

#include <IOKit/IOService.h>

#include "FakeSMCBinding.h"
#include "Binding.h"

// Registers
const UInt8 SUPERIO_CONFIGURATION_CONTROL_REGISTER	= 0x02;
const UInt8 SUPERIO_DEVCIE_SELECT_REGISTER			= 0x07;
const UInt8 SUPERIO_CHIP_ID_REGISTER				= 0x20;
const UInt8 SUPERIO_CHIP_REVISION_REGISTER			= 0x21;
const UInt8 VENDOR_ID_BYTE1_REG						= 0x23;
const UInt8 VENDOR_ID_BYTE2_REG						= 0x24;
const UInt8 SUPERIO_BASE_ADDRESS_REGISTER			= 0x60;

enum ChipModel
{
	UnknownModel = 0,
	
    IT8712F = 0x8712,
    IT8716F = 0x8716,
    IT8718F = 0x8718,
    IT8720F = 0x8720,
    IT8726F = 0x8726,
};

class SuperIO
{
private:
	Binding*		m_Binding;
protected:
	IOService*		m_Service;
	bool			m_FanControl;
	
	UInt16			Address;
	UInt8			RegisterPort;
	UInt8			ValuePort;
	
	ChipModel		Model;
	
	UInt16			LastVcore;
	
	UInt8			FanOffset;
	UInt8			FanCount;
	const char*		FanName[5];
	UInt8			FanIndex[5];

	void			Bind(Binding* binding);
	void			FlushBindings();
	
	UInt8			ListenPortByte(UInt8 reg);
	UInt16			ListenPortWord(UInt8 reg);
	void			Select(UInt8 logicalDeviceNumber);
	
public:
	IOService*		GetService() { return m_Service; };
	const char*		GetModelName();
	UInt16			GetAddress() { return Address; };
	
	virtual void	LoadConfiguration(IOService* provider);
	
	virtual UInt8	ReadByte(...) { return 0; };
	virtual UInt16	ReadWord(...) { return 0; };
	virtual SInt16	ReadTemperature(...) { return 0; };
	virtual SInt16	ReadVoltage(...) { return 0; };
	virtual SInt16	ReadTachometer(...) { return 0; };
	
	virtual bool	Probe() { return false; };
	virtual void	Init() {};
	virtual void	Finish() {};
};

#endif