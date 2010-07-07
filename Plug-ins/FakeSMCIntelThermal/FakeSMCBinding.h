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

class FakeSMCBinding 
{
public:
	virtual IOReturn OnKeyRead(__unused const char* key, __unused char* data) {return kIOReturnInvalid;};
	virtual IOReturn OnKeyWrite(__unused const char* key, __unused char* data) {return kIOReturnInvalid;};
};

void FakeSMCAddKey (const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, uint8_t, char*, FakeSMCBinding*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*, FakeSMCBinding*);
char* FakeSMCReadKey (const char*);
void FakeSMCRemoveKeyBinding (const char*);

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