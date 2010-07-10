/*
 *  FakeSMCBinding.h
 *  FakeSMC
 *
 *  Created by mozo on 06/07/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

class FakeSMCBinding 
{
public:
	virtual IOReturn OnKeyRead(const char* key, char* data)
	{
		return kIOReturnInvalid;
	};
	
	virtual IOReturn OnKeyWrite(const char* key, char* data)
	{
		return kIOReturnInvalid;
	};
};