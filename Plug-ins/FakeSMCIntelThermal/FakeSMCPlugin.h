/*
 *  FakeSMCPlugin.h
 *  fakesmc
 *
 *  Created by Mozodojo on 11/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef FAKESMCPLUGIN_H
#define FAKESMCPLUGIN_H

class FakeSMCPlugin 
{
public:
	virtual void OnKeyRead(const char* key, char* data);
	virtual void OnKeyWrite(const char* key, char* data);
};

void FakeSMCAddKey (const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*);
void FakeSMCAddKey (const char*, uint8_t, char*, FakeSMCPlugin*);
void FakeSMCAddKey (const char*, const char*, uint8_t, char*, FakeSMCPlugin*);
char* FakeSMCReadKey (const char*);
void FakeSMCRemoveKeyBinding (const char*);

inline bool CompareKeys(const char* key1, const char* key2)
{
	return ((key1[0] == key2[0]) && (key1[1] == key2[1]) && (key1[2] == key2[2]) && (key1[3] == key2[3]));
};

inline int GetIndex(char value)
{
	if (value >= 'A') 
		return value - 55;
	
	return value - 48;
};

#endif