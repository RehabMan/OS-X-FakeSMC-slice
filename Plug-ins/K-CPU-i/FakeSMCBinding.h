/*
 *  FakeSMCBinding.h
 *  fakesmc
 *
 *  Created by Mozodojo on 11/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef FAKESMCPLUGIN_H
#define FAKESMCPLUGIN_H

#include <architecture/i386/pio.h>
#include <libkern/OSTypes.h>
#include <IOKit/IOLib.h>

#ifndef DebugOn
#define DebugOn TRUE
#endif

#ifndef LogPrefix
#define LogPrefix "CPU-i: "
#endif

#ifndef DebugLog
#define DebugLog(string, args...)	do { if (DebugOn) { IOLog (LogPrefix string "\n", ## args); } } while(0)
#endif

#ifndef WarningLog
#define WarningLog(string, args...) do { IOLog (LogPrefix string "\n", ## args); } while(0)
#endif

#ifndef InfoLog
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)
#endif

class FakeSMCBinding 
{
public:
	virtual IOReturn OnKeyRead(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
	
	virtual IOReturn OnKeyWrite(__unused const char* key, __unused char* data)
	{
		return kIOReturnInvalid;
	};
};


void FakeSMCAddKey (const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, uint8_t, char*, FakeSMCBinding*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*, FakeSMCBinding*);
char* FakeSMCReadKey (const char*);
void FakeSMCRemoveKeyBinding (const char*);

class FrequencySensor : public FakeSMCBinding
{
protected:
	char*			m_Key;
public:
	FrequencySensor(){};
	
	FrequencySensor(const char* key, const char* type, UInt8 size){
		//InfoLog("Binding key %s", key);
		
		m_Key = (char*)IOMalloc(5);
		bcopy(key, m_Key, 5);
		
		char* value = (char*)IOMalloc(size);
		FakeSMCAddKey(key, type, size, value, this);
		IOFree(value, size);
	}
	
	~FrequencySensor() {
		if (m_Key) {
			//InfoLog("Removing key %s binding", m_Key);
			FakeSMCRemoveKeyBinding(m_Key);
			IOFree(m_Key, 5);
		}
	}
	
	virtual IOReturn OnKeyRead(const char* key, char* data);
	virtual IOReturn OnKeyWrite(const char* key, char* data){return kIOReturnSuccess;}
};


class TemperatureSensor : public FakeSMCBinding
{
protected:
	char*			m_Key;
public:
	TemperatureSensor(){};
	
	TemperatureSensor(const char* key, const char* type, UInt8 size){
		//InfoLog("Binding key %s", key);
		
		m_Key = (char*)IOMalloc(5);
		bcopy(key, m_Key, 5);
		
		char* value = (char*)IOMalloc(size);
		FakeSMCAddKey(key, type, size, value, this);
		IOFree(value, size);
	}
	
	~TemperatureSensor() {
		if (m_Key) {
			//InfoLog("Removing key %s binding", m_Key);
			FakeSMCRemoveKeyBinding(m_Key);
			IOFree(m_Key, 5);
		}
	}
	
	virtual IOReturn OnKeyRead(const char* key, char* data);
	virtual IOReturn OnKeyWrite(const char* key, char* data){return kIOReturnSuccess;}
};



inline UInt16 fp2e_Encode(UInt16 value)
{
	UInt16 dec = (float)value / 1000.0f;
	UInt16 frc = value - (dec * 1000);
	
	return (dec << 14) | (frc << 4) | 0xb;
}

inline bool CompareKeys(const char* key1, const char* key2) //strcmp?
{
	return ((key1[0] == key2[0]) && (key1[1] == key2[1]) && (key1[2] == key2[2]) && (key1[3] == key2[3]));
}

inline int GetIndex(const char* value, int index)
{
	if (value[index] >= 'A') 
		return 10 + value[index] - 'A';
	
	return value[index] - '0';
}

inline UInt8 GetFNum()
{
	char* data = FakeSMCReadKey("FNum");
	
	if(data)
		return data[0];
	
	return 0;
}

inline void UpdateFNum(UInt8 valueToAdd)
{
	char* data = FakeSMCReadKey("FNum");
	
	if(!data)
	{
		char data[1];
		
		data[0] = valueToAdd;
		FakeSMCAddKey("FNum", 1, data);
		
		return;
	}
	else
	{
		UInt8 oldValue = data[0];
		
		data[0] = oldValue + valueToAdd;
	}
}

#endif