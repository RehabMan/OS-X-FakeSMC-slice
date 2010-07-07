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
#include <IOKit/IOTimerEventSource.h>

#include "cpuid.h"
#include "FakeSMC.h"

// Ports
const UInt8 SUPERIO_STANDART_PORT[] = { 0x2e, 0x4e };

// Registers
const UInt8 SUPERIO_CONFIGURATION_CONTROL_REGISTER	= 0x02;
const UInt8 SUPERIO_DEVICE_SELECT_REGISTER			= 0x07;
const UInt8 SUPERIO_CHIP_ID_REGISTER				= 0x20;
const UInt8 SUPERIO_CHIP_REVISION_REGISTER			= 0x21;
const UInt8 SUPERIO_BASE_ADDRESS_REGISTER			= 0x60;

enum ChipModel
{
	UnknownModel = 0,
	
	// ITE
	IT8512F = 0x8512,
    IT8712F = 0x8712,
    IT8716F = 0x8716,
    IT8718F = 0x8718,
    IT8720F = 0x8720,
    IT8726F = 0x8726,
	IT8752F = 0x8752,
	
	// Winbond
	W83627DHG = 0xA020,
	W83627UHG = 0xA230,
    W83627DHGP= 0xB070,
    W83627EHF = 0x8800,    
    W83627HF  = 0x5200,
	W83627THF = 0x8283,
	W83627SF  = 0x5950,
	W83637HF  = 0x7080,
    W83667HG  = 0xA510,
    W83667HGB = 0xB350,
    W83687THF = 0x8541,
	W83697HF  = 0x6010,
	W83697SF  = 0x6810,
	
	//Slice
	/*W83977CTF = 0x5270,
	W83977EF  = 0x52F0,
	W83977FA  = 0x9771,
	W83977TF  = 0x9773,
	W83977ATF = 0x9774,
	W83977AF  = 0x9777,
	W83L517D  = 0x6100,
	W83877F   = 0xFA00,
	W83877AF  = 0xFB00,
	W83877TF  = 0xFC00,
	W83877ATF = 0xFD00,*/
	
	// Fintek
    F71858 = 0x0507,
    F71862 = 0x0601, 
    F71869 = 0x0814,
    F71882 = 0x0541,
    F71889ED = 0x0909,
    F71889F = 0x0723,
	
	// SMSC
	LPC47B27x = 0x51, // 0x0a
	LPC47B37x = 0x52, // 0x0a
	LPC47B397_NC = 0x6f, // 0x08
	LPC47M10x_112_13x = 0x59, // 0x0a
	LPC47M14x = 0x5f, // 0x0a
	LPC47M15x_192_997 = 0x60, // 0x0a
	LPC47M172 = 0x14, // 0x0a
	LPC47M182 = 0x74, // 0x0a
	LPC47M233 = 0x6b80, //mask: 0xff80, 0x0a
	LPC47M292 = 0x6b00, // 0xff80, 0x0a
	LPC47N252 = 0x0e, // 0x09
	LPC47N217 = 0x7a, 
	LPC47N227 = 0x5a, 
	LPC47S42x = 0x57, // 0x0a
	LPC47S45x = 0x62, // 0x0a
	LPC47U33x = 0x54, // 0x0a
	SCH3112 = 0x7c, // 0x0a
	SCH3114 = 0x7d, // 0x0a
	SCH3116 = 0x7f, // 0x0a
	SCH4307 = 0x90, // 0x08
	SCH5127 = 0x86, // 0x0a
	SCH5307_NS = 0x81, // 0x0a
	SCH5317_1 = 0x85, // 0x08
	SCH5317_2 = 0x8c // 0x08
};

class Binding : public FakeSMCBinding
{
public:
	Binding*	Next;
	
	virtual IOReturn OnKeyRead(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
	virtual IOReturn OnKeyWrite(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
};

class SuperIO : public FakeSMCBinding
{
private:
	Binding*			m_Sensor;
	Binding*			m_Controller;
	
	void			FlushList(Binding* start);
protected:
	IOService*		m_Service;
	
	UInt16			m_Address;
	UInt8			m_RegisterPort;
	UInt8			m_ValuePort;
	
	UInt16			m_Model;
	
	char*			m_TemperatureKey[3];
	
	UInt16			m_RawVCore;
	
	bool			m_FanControl;
	bool			m_FanVoltageControlled;
	
	UInt8			m_FanCount;
	const char*		m_FanName[5];
	UInt8			m_FanIndex[5];
	
	void			AddSensor(Binding* sensor);
	void			FlushSensors();
	
	void			AddController(Binding* controller);
	void			FlushControllers();
	
	UInt8			ListenPortByte(UInt8 reg);
	UInt16			ListenPortWord(UInt8 reg);
	void			Select(UInt8 logicalDeviceNumber);
	
	int				GetNextFanIndex();
	
public:
	IOService*			GetService() { return m_Service; };
	const char*			GetModelName();
	UInt16				GetAddress() { return m_Address; };
	Binding*			GetSensors() { return m_Sensor; };
	Binding*			GetControllers() { return m_Controller; };
	UInt16				GetRawVCore() { return m_RawVCore; };
	bool				FanControlEnabled() { return m_FanControl; };
	bool				FanVoltageControlled() { return m_FanVoltageControlled; };
	
	virtual IOReturn	OnKeyRead(const char* key, char* data);
	virtual IOReturn	OnKeyWrite(const char* key, char* data);
		
	virtual void		LoadConfiguration(IOService* provider);
	
	virtual UInt8		GetPortsCount() { return 2; };
	virtual void		SelectPort(UInt8 index) { m_RegisterPort = SUPERIO_STANDART_PORT[index]; m_ValuePort = SUPERIO_STANDART_PORT[index] + 1; };
	
	virtual void		Enter() {};
	virtual void		Exit() {};
	virtual bool		ProbePort() { return false; };
	
	virtual void		ControllersTimerEvent();
	
	virtual bool		Probe();
	virtual void		Start();
	virtual void		Stop();
};

class Sensor : public Binding
{
protected:
	SuperIO*	m_Provider;
	UInt8		m_Index;
	char*		m_Key;
	SInt32		m_Value;
public:
	
	Sensor(SuperIO* provider, UInt8 index, const char* key, const char* type, UInt8 size)
	{
		m_Provider = provider;
		m_Index = index;
		
		InfoLog("Binding key %s", key);
		
		m_Key = (char*)IOMalloc(5);
		bcopy(key, m_Key, 5);
		
		char* value = (char*)IOMalloc(size);
		FakeSMCAddKey(key, type, size, value, this);
		IOFree(value, size);
	};
	
	~Sensor()
	{
		if (m_Key)
		{
			InfoLog("Removing key %s binding", m_Key);
			FakeSMCRemoveKeyBinding(m_Key);
			IOFree(m_Key, 5);
		}
	}
	
	UInt8			GetIndex() { return m_Index; };
	const char*		GetKey() { return m_Key; };
	SInt32			GetValue() { return m_Value; }
	
	virtual IOReturn OnKeyRead(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
	 
	virtual IOReturn OnKeyWrite(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
};

class Controller : public Binding
{
public:
	virtual void TimerEvent() 
	{
		//
	};
	
	virtual IOReturn OnKeyRead(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
	 
	virtual IOReturn OnKeyWrite(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
};


#endif