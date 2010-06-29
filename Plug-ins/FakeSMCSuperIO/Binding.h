/*
 *  Binding.h
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 27/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#ifndef _BINDING_H
#define _BINDING_H

#include "BaseDefinitions.h"

class Binding : public FakeSMCBinding
{
protected:
	char*		m_Key;
public:
	Binding*	Next;
	
	Binding()
	{
	};
	
	Binding(const char* key, const char* type, UInt8 size)
	{		
		InfoLog("Binding key %s", key);
		
		m_Key = (char*)IOMalloc(5);
		bcopy(key, m_Key, 5);
		
		char* value = (char*)IOMalloc(size);
		FakeSMCAddKey(key, type, size, value, this);
		IOFree(value, size);
	};
	
	~Binding()
	{
		if (m_Key)
		{
			InfoLog("Removing key %s binding", m_Key);
			FakeSMCRemoveKeyBinding(m_Key);
			IOFree(m_Key, 5);
		}
	};
	
	const char* GetKey() 
	{ 
		return m_Key; 
	};
	
	virtual void OnKeyRead(__unused const char* key, __unused char* data)
	{
		// Or it will be link error on kextload
	};
	virtual void OnKeyWrite(__unused const char* key, __unused char* data)
	{
		// Or it will be link error on kextload
	};
};

#endif
