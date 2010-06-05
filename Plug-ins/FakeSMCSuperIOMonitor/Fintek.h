/*
 *  Fintek.h
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 31/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *  This code contains parts of original code from Open Hardware Monitor
 *  Copyright 2010 Michael Möller. All rights reserved.
 *
 */

#ifndef _FINTEK_H
#define _FINTEK_H 

#include "SuperIO.h"

// Hardware Monitor
const UInt8 FINTEK_ADDRESS_REGISTER_OFFSET = 0x05;
const UInt8 FINTEK_DATA_REGISTER_OFFSET = 0x06;

// Hardware Monitor Registers
const UInt8 FINTEK_VOLTAGE_BASE_REG = 0x20;
const UInt8 FINTEK_TEMPERATURE_CONFIG_REG = 0x69;
const UInt8 FINTEK_TEMPERATURE_BASE_REG = 0x70;
const UInt8 FINTEK_FAN_TACHOMETER_REG[] = { 0xA0, 0xB0, 0xC0, 0xD0 };

class Fintek : public SuperIO
{
private:
protected:
	UInt8	ReadByte(UInt8 reg);
	void	Enter();
	void	Exit();
	short	ReadTemperature(UInt8 index);
public:	
	virtual bool	Probe();
	virtual void	Init();
	virtual void	Finish();
	
	virtual void	Update(const char* key, char* data);
};

#endif