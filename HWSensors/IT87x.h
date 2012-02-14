/*
 *  IT87x.h
 *  HWSensors
 *
 *  Created by mozo on 08/10/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 *	Open Hardware Monitor Port
 *
 */

//Additional functionality added by Navi, inspired by FakeSMC development. Credits goes to Netkas, slice, Mozo, usr-sse2 and others...

#include <IOKit/IOService.h>
#include "SuperIOFamily.h"



#define KEY_FORMAT_FAN_TARGET_SPEED         "F%dTg"
// Old bad legacy naming but i have to keep it actually means

//￼￼￼￼￼￼Bit - 7 R/W Auto/Manual mode selection: 0 - software control, 1 - automatic chip control
//Bits 6-0 R/W Software PWM value... see description below
//FAN PWM mode Automatic/Software Operation Selection
//0: Software operation. 1: Automatic operation.
//128 steps of PWM control when in Software operation (bit 7=0), or Temperature input selection when in Automatic operation (bit 7=1). Bits[1:0]: - select temperature sensor to control the fan
//00: TMPIN1
//01: TMPIN2 
//10: TMPIN3 
//11: Reserved

//Smart guardian software mode. The fan speed will be determined by the PWM value entered into a register in it8718f by a software program. The pwm value is stored in bits 6-0 of a register. This is 0 for stopped and 127 for full speed.

//Smart guardian Automatic mode. The fan speed will be determined by the values in the it8718f registers.
#define KEY_FORMAT_FAN_MIN_SPEED            "F%dMn"
#define KEY_FORMAT_FAN_MAX_SPEED            "F%dMx"
#define KEY_FAN_FORCE                       "FS! "


#define KEY_FORMAT_FAN_START_TEMP           "F%dSt"
//start temperature, At this temperature the fan will start with the start pwm value. 
#define KEY_FORMAT_FAN_OFF_TEMP             "F%dSs"
//off temperature, At temperatures below this value the fan pwm value will be 0.   Usually 0 degrees is default value
#define KEY_FORMAT_FAN_FULL_TEMP            "F%dFt"
//Temperature limit when fan will run at max speed/PWM
#define KEY_FORMAT_FAN_START_PWM            "F%dPt"
//start PWM value, At start temperature this is the pwm value the fan will be running at. 
//Bit 7 - R/W Slope PWM bit[6]
//Please refer to the description of SmartGuardian Automatic Mode Control Register
//Bits 6-0 R/W Start PWM Value
#define KEY_FORMAT_FAN_TEMP_DELTA           "F%dFo"
//￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼￼Bit - 7 R/W Direct-Down Control
//This bit selects the PWM linear changing decreasing mode. 0: Slow decreasing mode. 1: Direct decreasing mode.
//Bits 6-5 - reserved
//￼￼￼￼￼￼￼Bit - ￼4-0 R/W  delta-Temperature interval [4:0].
//Direct-down control,  Direct decreasing mode. As temperature decreases the pwm value             will decrease  by the slope pwm value for each degree decrease.  Slow decreasing mode. As temperature decreases the pwm value will not decrease  until the temperature has decreased the value of  temperature interval. Then it will decrease by the slope pwm value.
//temperature interval, In Slow decreasing mode this is the value temperature has to decrease before  pwm value will decrease by slope pwm value. This is a 5 bit value, bits 4-0 of a register.

#define KEY_FORMAT_FAN_CONTROL              "F%dCt"
//Bit 7 R/W FAN Smoothing
//This bit enables the FAN PWM smoothing changing. 0: Disable
// 1: Enable
//Bit 6 R/W Reserved
//R/W Slope PWM bit[5:0]
//Slope = (Slope PWM bit[6:3] + Slope PWM bit[2:0] / 8) PWM value/°C 
//slope PWM value, At temperatures above start temperature, for each degree increase in 
//temperature, pwm value will increase by slope PWM value. This is an 7 bit value. 4 bits for 
//the whole number part and 3 bits  for the fractional part. this can be from 0 0/8 to 15 7/8.
//The fractional part is bits 2-0 of a register.The whole number part is bits 5-3 of a register 
//and bit 7 of another register.



const UInt8 ITE_ENVIRONMENT_CONTROLLER_LDN				= 0x04;

// ITE
const UInt8 ITE_VENDOR_ID								= 0x90;
const UInt8 ITE_VERSION_REGISTER                        = 0x22;

// ITE Environment Controller
const UInt8 ITE_ADDRESS_REGISTER_OFFSET					= 0x05;
const UInt8 ITE_DATA_REGISTER_OFFSET					= 0x06;

// ITE Environment Controller Registers    
const UInt8 ITE_CONFIGURATION_REGISTER					= 0x00;
const UInt8 ITE_TEMPERATURE_BASE_REG					= 0x29;
const UInt8 ITE_VENDOR_ID_REGISTER						= 0x58;
const UInt8 ITE_FAN_TACHOMETER_16_BIT_ENABLE_REGISTER	= 0x0c;
const UInt8 ITE_FAN_TACHOMETER_REG[5]					= { 0x0d, 0x0e, 0x0f, 0x80, 0x82 };
const UInt8 ITE_FAN_TACHOMETER_EXT_REG[5]				= { 0x18, 0x19, 0x1a, 0x81, 0x83 };
const UInt8 ITE_VOLTAGE_BASE_REG						= 0x20;

const float ITE_VOLTAGE_GAIN[]							= {1, 1, 1, (6.8f / 10 + 1), 1, 1, 1, 1, 1 };

const UInt8 ITE_SMARTGUARDIAN_MAIN_CONTROL				= 0x13;
const UInt8 ITE_SMARTGUARDIAN_PWM_CONTROL[5]			= { 0x15, 0x16, 0x17, 0x88, 0x89 };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_STOP[5]		= { 0x60, 0x68, 0x70, 0x90, 0x98 };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_START[5]		= { 0x61, 0x69, 0x71, 0x91, 0x99 };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_FULL_ON[5]	= { 0x62, 0x6a, 0x72, 0x92, 0x9a };
const UInt8 ITE_SMARTGUARDIAN_START_PWM[5]				= { 0x63, 0x6b, 0x73, 0x93, 0x9b };
const UInt8 ITE_SMARTGUARDIAN_CONTROL[5]				= { 0x64, 0x6c, 0x74, 0x94, 0x9c };
const UInt8 ITE_SMARTGUARDIAN_TEMPERATURE_DELTA[5]	= { 0x65, 0x6d, 0x75, 0x95, 0x9d };
// 

enum IT87xModel
{
	IT8512F = 0x8512,
    IT8712F = 0x8712,
    IT8716F = 0x8716,
    IT8718F = 0x8718,
    IT8720F = 0x8720,
    IT8721F = 0x8721,
    IT8726F = 0x8726,
	IT8728F = 0x8728,
	IT8752F = 0x8752,
    IT8772E = 0x8772
};

enum SuperIOSensorGroupEx {
	kSuperIOSmartGuardPWMControl = kSuperIOVoltageSensor +1,
	kSuperIOSmartGuardTempFanStop,
    kSuperIOSmartGuardTempFanStart,
    kSuperIOSmartGuardTempFanFullOn,
    kSuperIOSmartGuardPWMStart,
    kSuperIOSmartGuardTempFanFullOff,
    kSuperIOSmartGuardTempFanControl
};

inline UInt16 swap_value(UInt16 value)
{
	return ((value & 0xff00) >> 8) | ((value & 0xff) << 8);
}

inline UInt16 encode_fp2e(UInt16 value)
{
	UInt16 dec = (float)value / 1000.0f;
	UInt16 frc = value - (dec * 1000);
	
	return swap_value((dec << 14) | (frc << 4) /*| 0x3*/);
}

inline UInt16 encode_fpe2(UInt16 value)
{
	return swap_value(value << 2);
}

class IT87x;

class IT87xSensor : public SuperIOSensor
{
    OSDeclareDefaultStructors(IT87xSensor)
    


public:    
    static SuperIOSensor *withOwner(SuperIOMonitor *aOwner, const char* aKey, const char* aType, unsigned char aSize, SuperIOSensorGroup aGroup, unsigned long aIndex);

    virtual long		getValue();
    virtual void        setValue(UInt16 value);
};

class IT87x : public SuperIOMonitor
{
    OSDeclareDefaultStructors(IT87x)

	
private:
    long                    voltageGain;
    bool                    has16bitFanCounter;
    bool                    hasSmartGuardian;
//	UInt8					readByte(UInt8 reg);
//	UInt16					readWord(UInt8 reg1, UInt8 reg2);
//  void					writeByte(UInt8 reg, UInt8 value);
//    Not actually needed - better inline them
    

	
	virtual bool			probePort();
	virtual void			enter();
	virtual void			exit();
	
	virtual long			readTemperature(unsigned long index);
	virtual long			readVoltage(unsigned long index);
	virtual long			readTachometer(unsigned long index);
	

	virtual const char *	getModelName();
	
public:
	virtual bool			init(OSDictionary *properties=0);
	virtual IOService*		probe(IOService *provider, SInt32 *score);
    virtual bool			start(IOService *provider);
	virtual void			stop(IOService *provider);
	virtual void			free(void);
    
    virtual long			readSmartGuardPWMControl(unsigned long index);
    virtual long			readSmartGuardTempFanStop(unsigned long index);
    virtual long			readSmartGuardTempFanStart(unsigned long index);
    virtual long			readSmartGuardTempFanFullOn(unsigned long index);
    virtual long			readSmartGuardPWMStart(unsigned long index);
    virtual long			readSmartGuardTempFanFullOff(unsigned long index);
    virtual long			readSmartGuardFanControl(unsigned long index); 
//  New write SMC key value to SmartGuardian registers methods    
    virtual void			writeSmartGuardPWMControl(unsigned long index, UInt16 value);
    virtual void			writeSmartGuardTempFanStop(unsigned long index, UInt16 value);
    virtual void			writeSmartGuardTempFanStart(unsigned long index, UInt16 value);
    virtual void			writeSmartGuardTempFanFullOn(unsigned long index, UInt16 value);
    virtual void			writeSmartGuardPWMStart(unsigned long index, UInt16 value);
    virtual void			writeSmartGuardTempFanFullOff(unsigned long index, UInt16 value);
    virtual void			writeSmartGuardFanControl(unsigned long index, UInt16 value); 
    
    SuperIOSensor *			addSensor(const char* key, const char* type, unsigned char size, SuperIOSensorGroup group, unsigned long index);
    bool updateSensor(const char *key, const char *type, unsigned char size, SuperIOSensorGroup group, unsigned long index);

    
    virtual IOReturn callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 );
};

inline UInt8 readByte(UInt16 address, UInt8 reg)
{
	outb(address + ITE_ADDRESS_REGISTER_OFFSET, reg);
	
	UInt8 value = inb(address + ITE_DATA_REGISTER_OFFSET);	
	__unused UInt8 check = inb(address + ITE_DATA_REGISTER_OFFSET);
    return value;
    
}

inline UInt16 readWord(UInt16 address,UInt8 reg1, UInt8 reg2)
{	
	return readByte(address,reg1) << 8 | readByte(address,reg2);
}

inline void writeByte(UInt16 address,UInt8 reg, UInt8 value)
{
	outb(address + ITE_ADDRESS_REGISTER_OFFSET, reg);
	outb(address + ITE_DATA_REGISTER_OFFSET, value);
}
