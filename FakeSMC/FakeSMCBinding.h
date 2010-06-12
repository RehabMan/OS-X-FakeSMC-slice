/*
 *  FakeSMCBinding.h
 *  fakesmc
 *
 *  Created by Mozodojo on 11/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef FAKESMCBINDING_H
#define FAKESMCBINDING_H

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

#endif