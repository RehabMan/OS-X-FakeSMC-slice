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

#define DebugOn FALSE

#define LogPrefix "FakeSMC_Radeon: "
#define DebugLog(string, args...)	do { if (DebugOn) { IOLog (LogPrefix "[Debug] " string "\n", ## args); } } while(0)
#define WarningLog(string, args...) do { IOLog (LogPrefix "[Warning] " string "\n", ## args); } while(0)
#define InfoLog(string, args...)	do { IOLog (LogPrefix string "\n", ## args); } while(0)

#define	KEY_FORMAT_FAN_ID					"F%XID"
#define	KEY_FORMAT_FAN_SPEED				"F%XAc"
#define	KEY_FORMAT_GPU_DIODE_TEMPERATURE	"TG%XD"
#define	KEY_FORMAT_GPU_BOARD_TEMPERATURE	"TG%XH"

#define TYPE_FPE2	"fpe2"
#define TYPE_FP2E	"fp2e"
#define TYPE_CH8	"ch8*"
#define TYPE_SP78	"sp78"
#define TYPE_UI16	"ui16"

class FakeSMCBinding 
{
public:
	virtual IOReturn OnKeyRead(const char* key, char* data);
	virtual IOReturn OnKeyWrite(const char* key, char* data);
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

inline int GetNextUnusedKey(const char* format, char* key)
{
	for (UInt8 i = 0; i < 16; i++)
	{
		snprintf(key, 5, format, i);
		
		SMCData node = FakeSMCGetKey(key);
		
		if (node == NULL || node->binding == NULL)
			return i;
	}
	
	return -1;
}

inline int GetLastKeyIndex(const char* format, bool binding)
{
	char key[5];
	int offset = -1;
	
	for (UInt8 i = 0; i < 16; i++)
	{
		snprintf(key, 5, format, i);
		
		SMCData node = FakeSMCGetKey(key);
		
		if (binding)
		{
			if (node != NULL && node->binding != NULL) offset = i;
		}
		else 
		{
			if (node != NULL) offset = i;
		}
	}
	
	return offset;
}


inline void UpdateFNum()
{
	int idOffset = GetLastKeyIndex(KEY_FORMAT_FAN_ID, false);
	int acOffset = GetLastKeyIndex(KEY_FORMAT_FAN_SPEED, true);
	
	char data[1];
	
	data[0] = idOffset > acOffset ? idOffset : acOffset;
	data[0] += 1;
	
	FakeSMCAddKey("FNum", 1, data);
}


#endif