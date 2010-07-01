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


#include <architecture/i386/pio.h>
#include <libkern/OSTypes.h>
#include <IOKit/IOLib.h>

#define DebugOn FALSE

#define LogPrefix "SuperIO: "
#define DebugLog(string, args...)	do { if (DebugOn) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define KEY_CPU_HEATSINK_TEMPERATURE	"Th0H"
#define KEY_NORTHBRIDGE_TEMPERATURE		"TN0P"
#define KEY_CPU_VOLTAGE					"VC0C"

#define	KEY_FORMAT_FAN_ID				"F%xID"
#define	KEY_FORMAT_FAN_RPM				"F%xAc"

#define TYPE_FPE2	"fpe2"
#define TYPE_FP2E	"fp2e"
#define TYPE_CH8	"ch8*"
#define TYPE_SP78	"sp78"

class FakeSMCBinding 
{
public:
	virtual void OnKeyRead(const char* key, char* data);
	virtual void OnKeyWrite(const char* key, char* data);
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
SMCData FakeSMCGetKey (const char*);
void FakeSMCRemoveKeyBinding (const char*);

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

inline int GetNextUnusedKey(const char* format, char* value)
{
	for (UInt8 i = 0; i < 16; i++)
	{
		snprintf(value, 5, format, i);
		
		SMCData node = FakeSMCGetKey(value);
		
		if (node == NULL || node->binding == NULL)
			return i;
	}
	
	return -1;
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