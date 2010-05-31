/*
 *  SuperIO.h
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _SUPERIO_H 
#define _SUPERIO_H

#include <libkern/OSTypes.h>
#include <architecture/i386/pio.h>
#include <IOKit/IOLib.h>

const UInt8 SUPERIO_REGISTER_PORT[2] = { 0x2e, 0x4e };
const UInt8 SUPERIO_VALUE_PORT[2] = { 0x2f, 0x4f };

// Registers
const UInt8 SUPERIO_CONFIGURATION_CONTROL_REGISTER	= 0x02;
const UInt8 SUPERIO_DEVCIE_SELECT_REGISTER			= 0x07;
const UInt8 SUPERIO_CHIP_ID_REGISTER				= 0x20;
const UInt8 SUPERIO_CHIP_REVISION_REGISTER			= 0x21;
const UInt8 SUPERIO_BASE_ADDRESS_REGISTER			= 0x60;

enum SuperIOModel
{
	UnknownModel = 0,
	
    IT8712F = 0x8712,
    IT8716F = 0x8716,
    IT8718F = 0x8718,
    IT8720F = 0x8720,
    IT8726F = 0x8726,
	
	W83627DHG = 0xA020,
    W83627DHGP = 0xB070,
    W83627EHF = 0x8800,    
    W83627HF = 0x5200,
	W83627THF = 0x8283,
    W83667HG = 0xA510,
    W83667HGB = 0xB350,
    W83687THF = 0x8541,
	
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
protected:
	UInt8	RegisterPort;
	UInt8	ValuePort;
	UInt16	Address;

	void		SetPorts(UInt8 index);
	UInt8		ReadByte(UInt8 reg);
	UInt16		ReadWord(UInt8 reg);
	void		Select(UInt8 logicalDeviceNumber);
public:
	SuperIOModel	Model;
	
	virtual bool	Probe();
	virtual void	Init();
	virtual void	Finish();
	
	const char*		GetModelNameString();
};

#endif