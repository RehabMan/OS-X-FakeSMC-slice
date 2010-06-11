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
void FakeSMCAddKeyBinding (const char*, uint8_t, char*, FakeSMCPlugin*);
void FakeSMCAddKeyBinding (const char*, const char*, uint8_t, char*, FakeSMCPlugin*);
char* FakeSMCGetKeyData (const char*);
void FakeSMCRemoveKeyBinding (const char*);

#endif