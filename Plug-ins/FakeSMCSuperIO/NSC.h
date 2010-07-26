/*
 *  NSC.h
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#ifndef _NSC_H
#define _NSC_H 

#include "SuperIO.h"

const UInt8 NSC_PORTS_COUNT = 2;
const UInt16 NSC_PORT[2] = { 0x2e, 0x4e};
const UInt16 NSC_HARDWARE_MONITOR_REGS[5] = {0x61, 0x62, 0x63, 0x64, 0x68};
//const UInt16 NSC_FAN_SPEED[5] = {0xff, 0xa3, 0x82, 0x81, 0x6f, 0};  //bred

// ITE Environment Controller
const UInt8 NSC_ADDRESS_REGISTER_OFFSET = 0x00;
const UInt8 NSC_DATA_REGISTER_OFFSET	= 0x01;
const UInt8 NSC_BANK_SELECT_REGISTER	= 0x07;
const UInt8 NSC_CHIP_ID_REGISTER		= 0x20;
const UInt8 NSC_CHIP_REVISION_REGISTER	= 0x27;
const UInt8 NSC_LDN_PRESENT				= 0x30;


const UInt8 NSC_HARDWARE_MONITOR_LDN	= 0x0F;
const UInt8 NSC_MEM						= 0xF4;


// NSC Hardware Monitor Registers

class NSC : public SuperIO
{
private:
	volatile UInt8* mmio_base;
	IOMemoryMap *	mmio;
	
private:
	UInt16	m_FanValue[5];
	bool	m_FanValueObsolete[5];
	int		FanCount;
public:
	void			WriteByte(UInt8 bank, UInt8 reg, UInt8 value);
	
	UInt8			ReadByte(UInt8 bank, UInt8 reg);
	SInt16			ReadTemperature(UInt8 index);
	SInt16			ReadVoltage(UInt8 index);
	SInt16			ReadTachometer(UInt8 index);

	virtual void	Enter();
	virtual void	Exit();

	virtual bool	ProbePort();
//	virtual void	Init();
	virtual void	Finish();
	virtual void	Start();
};

#endif