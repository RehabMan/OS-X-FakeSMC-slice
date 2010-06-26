/*
 *  FakeSMCBinding.h
 *  fakesmc
 *
 *  Created by Mozodojo on 11/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _BASEDEFINITIONS_H
#define _BASEDEFINITIONS_H

#define DebugOn FALSE

#define LogPrefix "FakeSMCSuperIO: "
#define DebugLog(string, args...)	do { if (DebugOn) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#include <architecture/i386/pio.h>
#include <libkern/OSTypes.h>
#include <IOKit/IOLib.h>

class FakeSMCBinding 
{
public:
	virtual void OnKeyRead(const char* key, char* data);
	virtual void OnKeyWrite(const char* key, char* data);
};

void FakeSMCAddKey (const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, uint8_t, char*, FakeSMCBinding*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*, FakeSMCBinding*);
char* FakeSMCReadKey (const char*);
void FakeSMCRemoveKeyBinding (const char*);

class Binding : public FakeSMCBinding
{
protected:
	char*			m_Key;
public:
	Binding*		Next;
	
	Binding()
	{
	};
	
	Binding(const char* key, const char* type, UInt8 size)
	{
		InfoLog("Binding key %s", key);
		
		m_Key = (char*)IOMalloc(5);
		bcopy(key, m_Key, 5);
		
		char* value = (char*)IOMalloc(size);
		FakeSMCAddKey(key, type, size, value, this);
		IOFree(value, size);
	};
	
	~Binding()
	{
		if (m_Key)
		{
			InfoLog("Removing key %s binding", m_Key);
			FakeSMCRemoveKeyBinding(m_Key);
			IOFree(m_Key, 5);
		}
	};
	
	const char* GetKey() 
	{ 
		return m_Key; 
	};
	
	virtual void OnKeyRead(__unused const char* key, __unused char* data)
	{
		// Or it will be link error on kextload
	};
	virtual void OnKeyWrite(__unused const char* key, __unused char* data)
	{
		// Or it will be link error on kextload
	};
};

class Controller
{
public:
	Controller*	Next;
	
	virtual void TimerEvent() {};
};

inline UInt16 fp2e_Encode(UInt16 value)
{
	UInt16 dec = (float)value / 1000.0f;
	UInt16 frc = value - (dec * 1000);
	
	return (dec << 14) | (frc << 4) | 0xb;
}

inline bool CompareKeys(const char* key1, const char* key2)
{
	return ((key1[0] == key2[0]) && (key1[1] == key2[1]) && (key1[2] == key2[2]) && (key1[3] == key2[3]));
}

inline int GetIndex(const char* value, int index)
{
	if (value[index] >= 'A') 
		return value[index] - 55;
	
	return value[index] - 48;
}

inline UInt8 GetFNum()
{
	char* data = FakeSMCReadKey("FNum");
	
	if(data != NULL)
	{
		return data[0];
	}
	
	return 0;
}

inline void UpdateFNum(UInt8 valueToAdd)
{
	char* data = FakeSMCReadKey("FNum");
	
	if(data == NULL)
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