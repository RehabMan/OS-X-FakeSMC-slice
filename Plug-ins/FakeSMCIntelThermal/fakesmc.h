/*
 *  fakesmc.h
 *  FakeSMCIntelThermal
 *
 *  Created by mozodojo on 18/05/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

typedef void (*OnKeyReadCallback)(const char*, char*);

void FakeSMCAddKey (const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*);
void FakeSMCAddKeyCallback (const char*, uint8_t, char*, OnKeyReadCallback);
void FakeSMCAddKeyCallback (const char*, const char*, uint8_t, char*, OnKeyReadCallback);
void FakeSMCRemoveKeyCallback (const char*);

inline bool CompareKeys(const char* key1, const char* key2)
{
	return ((key1[0] == key2[0]) && (key1[1] == key2[1]) && (key1[2] == key2[2]) && (key1[3] == key2[3]));
}

inline UInt8 GetIndex(char value)
{
	if (value >= 'A') 
		return value - 55;
	
	return value - 48;
}