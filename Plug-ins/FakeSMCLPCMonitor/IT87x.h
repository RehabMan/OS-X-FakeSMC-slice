/*
 *  IT87x.h
 *  FakeSMCLPCMonitor
 *
 *  Created by Mozodojo on 29/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */
#ifndef IT87X_H 
#define IT87X_H

#include "SuperIO.h"

const unsigned char IT87_ENVIRONMENT_CONTROLLER_LDN = 0x04;

// ITE
const unsigned char ITE_VENDOR_ID = 0x90;

// ITE Environment Controller
const unsigned char ITE_ADDRESS_REGISTER_OFFSET = 0x05;
const unsigned char ITE_DATA_REGISTER_OFFSET = 0x06;

// ITE Environment Controller Registers    
const unsigned char ITE_CONFIGURATION_REGISTER = 0x00;
const unsigned char ITE_TEMPERATURE_BASE_REG = 0x29;
const unsigned char ITE_VENDOR_ID_REGISTER = 0x58;
const unsigned char ITE_FAN_TACHOMETER_16_BIT_ENABLE_REGISTER = 0x0c;
const unsigned char ITE_FAN_TACHOMETER_REG[] = { 0x0d, 0x0e, 0x0f, 0x80, 0x82 };
const unsigned char ITE_FAN_TACHOMETER_EXT_REG[] = { 0x18, 0x19, 0x1a, 0x81, 0x83 };
const unsigned char ITE_VOLTAGE_BASE_REG = 0x20;

const float ITE_VOLTAGE_GAIN[] = {1, 1, 1, (6.8f / 10 + 1), 1, 1, 1, 1, 1 };

class IT87x : public SuperIO
{
private:
protected:
public:
	UInt16	LastVcore;
	
	UInt8	FanOffset;
	UInt8	FanCount;
	UInt8	FanIndex[5];
	
	UInt8 	ReadByte(UInt8 reg, bool* valid);
	UInt16	ReadRPM(UInt8 num);
	void	Enter();
	void	Exit();
	
	virtual bool	Probe();
	virtual void	Init();
	virtual void	Finish();
	
	void Update(const char* key, char* data);
};

#endif