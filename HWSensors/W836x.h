/*
 *  W836x.h
 *  HWSensors
 *
 *  Created by mozo on 14/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *	Open Hardware Monitor Port
 *
 */

#include <IOKit/IOService.h>
#include "SuperIOFamily.h"

const UInt8 WINBOND_HARDWARE_MONITOR_LDN			= 0x0B;

const UInt16 WINBOND_VENDOR_ID						= 0x5CA3;
const UInt8 WINBOND_HIGH_BYTE						= 0x80;

// Winbond Hardware Monitor
const UInt8 WINBOND_ADDRESS_REGISTER_OFFSET			= 0x05;
const UInt8 WINBOND_DATA_REGISTER_OFFSET			= 0x06;

// Winbond Hardware Monitor Registers
const UInt8 WINBOND_BANK_SELECT_REGISTER			= 0x4E;
//const UInt8 WINBOND_VENDOR_ID_REGISTER			= 0x4F;
const UInt8 WINBOND_TEMPERATURE_SOURCE_SELECT_REG	= 0x49;

//private string[] TEMPERATURE_NAME = 
//new string[] {"CPU", "Auxiliary", "System"};
const UInt8 WINBOND_TEMPERATURE[]					= { 0x50, 0x50, 0x27 };
const UInt8 WINBOND_TEMPERATURE_BANK[]				= { 1,    2,    0 };

const UInt8 WINBOND_VOLTAGE							= 0x20;

const UInt8 WINBOND_TACHOMETER[]					= { 0x28, 0x29, 0x2A, 0x3F, 0x53 };
const UInt8 WINBOND_TACHOMETER_BANK[]				= { 0, 0, 0, 0, 5 };

const UInt8 WINBOND_TACHOMETER_DIV0[]				= { 0x47, 0x47, 0x4B, 0x59, 0x59 };
const UInt8 WINBOND_TACHOMETER_DIV0_BIT[]			= { 4,    6,    6,    0,    2 };
const UInt8 WINBOND_TACHOMETER_DIV1[]				= { 0x47, 0x47, 0x4B, 0x59, 0x59 };
const UInt8 WINBOND_TACHOMETER_DIV1_BIT[]			= { 5,    7,    7,    1,    3 };
const UInt8 WINBOND_TACHOMETER_DIV2[]				= { 0x5D, 0x5D, 0x5D, 0x4C, 0x59 };
const UInt8 WINBOND_TACHOMETER_DIV2_BIT[]			= { 5,    6,    7,    7,    7 };

const UInt8 WINBOND_TACHOMETER_DIVISOR[]			= { 0x47, 0x4B, 0x4C, 0x59, 0x5D };
const UInt8 WINBOND_TACHOMETER_DIVISOR0[]			= { 36, 38, 30, 8, 10 };
const UInt8 WINBOND_TACHOMETER_DIVISOR1[]			= { 37, 39, 31, 9, 11 };
const UInt8 WINBOND_TACHOMETER_DIVISOR2[]			= { 5, 6, 7, 23, 15 };

// Fan Control
const UInt8 WINBOND_FAN_CONFIG[]					= { 0x04, 0x04, 0x12, 0x62 };
const UInt8 WINBOND_FAN_CONTROL_BIT[]				= { 0x02, 0x04, 0x01, 0x04 };
const UInt8 WINBOND_FAN_MODE_BIT[]					= { 0x00, 0x01, 0x00, 0x06 };
const UInt8 WINBOND_FAN_OUTPUT[]					= { 0x01, 0x03, 0x11, 0x61 };

enum W836xModel
{
	W83627DHG	= 0xA020,
	W83627UHG	= 0xA230,
    W83627DHGP	= 0xB070,
    W83627EHF	= 0x8800,    
    W83627HF	= 0x5200,
	W83627THF	= 0x8280,
	W83627SF	= 0x5950,
	W83637HF	= 0x7080,
    W83667HG	= 0xA510,
    W83667HGB	= 0xB350,
    W83687THF	= 0x8541,
	W83697HF	= 0x6010,
	W83697SF	= 0x6810
};

class W836x : public SuperIOMonitor
{
    OSDeclareDefaultStructors(W836x)
	
private:
	UInt8					fanLimit;
	UInt16					fanValue[5];
	bool					fanValueObsolete[5];
	
	void					writeByte(UInt8 bank, UInt8 reg, UInt8 value);
	UInt8					readByte(UInt8 bank, UInt8 reg);
	
	virtual bool			probePort();
	virtual void			enter();
	virtual void			exit();
	
	virtual long			readTemperature(unsigned long index);
	virtual long			readVoltage(unsigned long index);
	void					updateTachometers();
	virtual long			readTachometer(unsigned long index);
	
	virtual const char *	getModelName();
	
public:
	virtual bool			init(OSDictionary *properties=0);
	virtual IOService*		probe(IOService *provider, SInt32 *score);
    virtual bool			start(IOService *provider);
	virtual void			stop(IOService *provider);
	virtual void			free(void);
};