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

struct AppleSMCData;

typedef struct AppleSMCData* SMCData;

struct AppleSMCData {
	uint8_t len;
	char *key;
	char *type;
	char *data;
	FakeSMCBinding* binding;
	SMCData next;
};

void FakeSMCAddKey (const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, uint8_t, char*, FakeSMCBinding*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*, FakeSMCBinding*);
char* FakeSMCReadKey (const char*);
void FakeSMCRemoveKeyBinding (const char*);
SMCData FakeSMCGetKey (const char*);


inline UInt16 fp2e_Encode(UInt16 value)
{
	UInt16 dec = (float)value / 1000.0f;
	UInt16 frc = value - (dec * 1000);
	
	return (dec << 14) | (frc << 4) | 0xb;
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


inline bool CompareKeys(const char* key1, const char* key2)
{
	uint32_t key1_t = *((uint32_t*)key1);
	uint32_t key2_t = *((uint32_t*)key2);
	
	return key1_t == key2_t;
}

inline int GetIndex(char value)
{
	if (value >= 'A') 
		return value - 55;
	
	return value - 48;
};

#endif