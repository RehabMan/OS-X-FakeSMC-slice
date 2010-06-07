/*
 *  fakesmc.h
 *  FakeSMCIntelThermal
 *
 *  Created by mozodojo on 18/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _FAKESMC_H 
#define _FAKESMC_H

typedef void (*OnKeyReadCallback)(const char* key, char* data);
typedef void (*OnKeyWriteCallback)(const char* key, char* data, bool* drop);

void FakeSMCAddKey (const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*);
void FakeSMCAddKeyCallback (const char*, uint8_t, char*, OnKeyReadCallback);
void FakeSMCAddKeyCallback (const char*, uint8_t, char*, OnKeyReadCallback, OnKeyWriteCallback);
void FakeSMCAddKeyCallback (const char*, const char*, uint8_t, char*, OnKeyReadCallback);
void FakeSMCAddKeyCallback (const char*, const char*, uint8_t, char*, OnKeyReadCallback, OnKeyWriteCallback);
char* FakeSMCGetKey (const char*);
void FakeSMCRemoveKeyCallback(const char*);

static bool CompareKeys(const char* key1, const char* key2)
{
	return ((key1[0] == key2[0]) && (key1[1] == key2[1]) && (key1[2] == key2[2]) && (key1[3] == key2[3]));
}

static UInt8 GetFNum()
{
	char* data = FakeSMCGetKey("FNum");
	
	if(data != NULL)
	{
		return data[0];
	}
	
	return 0;
}

static void UpdateFNum(UInt8 valueToAdd)
{
	char* data = FakeSMCGetKey("FNum");
	
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