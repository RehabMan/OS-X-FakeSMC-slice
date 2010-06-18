/*
 *  SmartGuardianController.cpp
 *  TheOnlyWorkingITESMCController
 *
 *  Created by Mozodojo on 13/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "SmartGuardianController.h"

void SmartGuardianController::ForcePWM(UInt8 slope)
{
	DebugLog("Forcing Fan %d to %d%%", m_Offset, slope*100/127);
	
	outb(m_Provider->GetAddress() + ITE_ADDRESS_REGISTER_OFFSET, ITE_SMARTGUARDIAN_FORCE_PWM[m_Offset]);
	outb(m_Provider->GetAddress() + ITE_DATA_REGISTER_OFFSET, slope);
}

void SmartGuardianController::Initialize()
{
	bool* valid;
	char tmpKey[5];
	
	//Back up temperature sensor selection
	m_DefaultForcePWM = m_Provider->ReadByte(ITE_SMARTGUARDIAN_FORCE_PWM[m_Offset], valid);
	m_DefaultStartPWM = m_Provider->ReadByte(ITE_SMARTGUARDIAN_START_PWM[m_Offset], valid);
	
	if (valid)
	{
		char value[2];
		
		UInt16 initial = m_Maximum = m_Provider->ReadTachometer(m_Offset);
		
		//Forcing maximum speed
		ForcePWM(0x7f);
		
		UInt16 last = initial, count = 0;
		
		//Waiting cooler will speed up to maximum
		while (count < 5)
		{
			IOSleep(1000);
			
			m_Maximum = m_Provider->ReadTachometer(m_Offset);
			
			if (m_Maximum < last + 50)
			{
				count++;
			}
			else 
			{
				last = m_Maximum;
			}
		}
		
		//Forcing minimum speed
		ForcePWM(0x00);
		
		count = 0;
		
		//Waiting cooler will slow down to minimum
		while (count < 5)
		{
			IOSleep(1000);
			
			m_Minimum = m_Provider->ReadTachometer(m_Offset);
			
			if (m_Minimum > last - 50)
			{
				count++;
			}
			else 
			{
				last = m_Minimum;
			}
		}
		
	
		//Restore temperature sensor selection
		ForcePWM(m_DefaultForcePWM);
			
		DebugLog("Fan #%d MAX=%drpm MIN=%drpm", m_Offset, m_Maximum, m_Minimum);
		
		value[0] = (m_Maximum << 2) >> 8;
		value[1] = (m_Maximum << 2) & 0xff;
				
		snprintf(tmpKey, 5, "F%dMx", m_Index);
		FakeSMCAddKey(tmpKey, "fpe2", 2, value);
		
		
		
		if (m_Maximum > initial + 100)
		{
			value[0] = (m_Minimum << 2) >> 8;
			value[1] = (m_Minimum << 2) & 0xff;
					
			m_Key = (char*)IOMalloc(5);
			snprintf(m_Key, 5, "F%dMn", m_Index);
					
			InfoLog("Binding key %s", m_Key);
					
			FakeSMCAddKey(m_Key, "fpe2", 2, value, this);
		}
		else 
		{
			value[0] = (initial << 2) >> 8;
			value[1] = (initial << 2) & 0xff;
			
			m_Key = (char*)IOMalloc(5);
			snprintf(m_Key, 5, "F%dMn", m_Index);
			
			FakeSMCAddKey(m_Key, "fpe2", 2, value);
		}
	}	
}

void SmartGuardianController::OnKeyRead(__unused const char* key, __unused char* data)
{
}

void SmartGuardianController::OnKeyWrite(__unused const char* key, char* data)
{
	if (m_Maximum>m_Minimum){
		UInt16 rpm = (UInt16(data[0] << 8) | (data[1] & 0xff)) >> 2;
		UInt8 slope;
		if (rpm<m_Minimum)
			slope=0;
		else if (rpm>m_Maximum)
			slope=0x7f;
		else
			slope = (rpm-m_Minimum) * 127 / (m_Maximum-m_Minimum);
		//DebugLog("Fan %d start slope=0x%x", m_Offset, slope, rpm);
		
		outb(m_Provider->GetAddress() + ITE_ADDRESS_REGISTER_OFFSET, ITE_SMARTGUARDIAN_START_PWM[m_Offset]);
		outb(m_Provider->GetAddress() + ITE_DATA_REGISTER_OFFSET, slope);
	}
}