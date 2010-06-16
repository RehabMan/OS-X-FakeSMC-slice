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
	
	IT8512F = 0x8512,
    IT8712F = 0x8712,
    IT8716F = 0x8716,
    IT8718F = 0x8718,
    IT8720F = 0x8720,
    IT8726F = 0x8726,
	IT8752F = 0x8752,
	
	W83627DHG = 0xA020,
    W83627DHGP= 0xB070,
    W83627EHF = 0x8800,    
    W83627HF  = 0x5200,
	W83627THF = 0x8283,
    W83667HG  = 0xA510,
    W83667HGB = 0xB350,
    W83687THF = 0x8541,
	//Slice
	W83977CTF = 0x5270,
	W83977EF  = 0x52F0,
	W83977FA  = 0x9771,
	W83977TF  = 0x9773,
	W83977ATF = 0x9774,
	W83977AF  = 0x9777,
	W83627SF  = 0x5950,
	W83697HF  = 0x6010,
	W83L517D  = 0x6100,
	W83637HF  = 0x7080,
	W83627UHG = 0xA230,
	W83697SF  = 0x6810,
	W83877F   = 0xFA00,
	W83877AF  = 0xFB00,
	W83877TF  = 0xFC00,
	W83877ATF = 0xFD00,
	
	
    F71858 = 0x0507,
    F71862 = 0x0601, 
    F71869 = 0x0814,
    F71882 = 0x0541,
    F71889ED = 0x0909,
    F71889F = 0x0723 
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