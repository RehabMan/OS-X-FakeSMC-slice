/*
 *  SMSC.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 22/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _SMSC_H
#define _SMSC_H

#include "SuperIO.h"

const UInt8 SMSC_PORTS_COUNT = 2;
const UInt16 SMSC_PORT[] = { 0x2e, 0x4e };

const UInt8 SMSC_HARDWARE_MONITOR_LDN		= 0x0a;
const UInt8 LPC47N252_HARDWARE_MONITOR_LDN	= 0x09;   //???
const UInt8 SMSC_HARDWARE_MONITOR_LDN_VA	= 0x08;  //???

const UInt8 SMSC_LDN						= 0x07;

const UInt8 SMSC_FAN1						= 0x56;
const UInt8 SMSC_FAN2						= 0x57;
const UInt8 SMSC_FAN_CONTROL				= 0x58;
const UInt8 SMSC_FAN1_TACH					= 0x59;
const UInt8 SMSC_FAN2_TACH					= 0x5A;
const UInt8 SMSC_FAN1_PRELOAD				= 0x5B;
const UInt8 SMSC_FAN2_PRELOAD				= 0x5B;

class SMSC : public SuperIO
{
public:	
	void	WriteByte(UInt8 bank, UInt8 reg, UInt8 value);
	
	UInt8	ReadByte(UInt8 bank, UInt8 reg);
//	SInt16	ReadTemperature(UInt8 index);
//	SInt16	ReadVoltage(UInt8 index);
	SInt16	ReadTachometer(UInt8 index, bool force_update);
	
	
	virtual void	Enter();
	virtual void	Exit();
	
	virtual bool	ProbePort();
	
	virtual void	Start();
};

#endif