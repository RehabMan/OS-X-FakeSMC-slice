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
const UInt8 LPC47N252_HARDWARE_MONITOR_LDN	= 0x09; //Some chips has diferent FAN logical device number
const UInt8 SMSC_HARDWARE_MONITOR_LDN_VA	= 0x08;  

const UInt8 SMSC_LDN						= 0x07;

const UInt8 SMSC_FAN_PWM[]					= { 0x56, 0x57, 0x69 };
const UInt8 SMSC_FAN_TACHOMETER[]			= { 0x59, 0x5a, 0x6b };
const UInt8 SMSC_FAN_PRELOAD[]				= { 0x5b, 0x5c, 0x6c };
const UInt8 SMSC_FAN_CONTROL				= 0x58;

class SMSC : public SuperIO
{
private:
	UInt8	m_FanLimit;
	
public:	
	void	WriteByte(/*UInt8 bank,*/ UInt8 reg, UInt8 value);
	UInt8	ReadByte(/*UInt8 bank,*/ UInt8 reg);

	SInt16	ReadTachometer(UInt8 index);
	
	virtual void	Enter();
	virtual void	Exit();
	
	virtual bool	ProbePort();
	
	virtual void	Start();
};

#endif