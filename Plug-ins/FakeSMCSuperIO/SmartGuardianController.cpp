/*
 *  SmartGuardianController.cpp
 *  FakeSMCSuperIOMonitor
 *
 *  Created by Mozodojo on 13/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "SmartGuardianController.h"

void SmartGuardianController::ForcePWM(UInt8 slope)
{
	DebugLog("Forcing Fan #%d SLOPE=0x%x", m_Offset, slope);
	
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
		};
	
		//Restore temperature sensor selection
		ForcePWM(m_DefaultForcePWM);
			
		m_Maximum = m_Maximum / 50 * 50;
		
		DebugLog("Fan #%d MAX=%drpm", m_Offset, m_Maximum);
		
		value[0] = (m_Maximum << 2) >> 8;
		value[1] = (m_Maximum << 2) & 0xff;
				
		snprintf(tmpKey, 5, "F%dMx", m_Index);
		FakeSMCAddKey(tmpKey, "fpe2", 2, value);
		
		initial = initial / 50 * 50;
		
		value[0] = (initial << 2) >> 8;
		value[1] = (initial << 2) & 0xff;
		
		snprintf(tmpKey, 5, "F%dTg", m_Index);
		FakeSMCAddKey(tmpKey, "fpe2", 2, value);
				
		if (m_Maximum > initial + 100)
		{
			value[0] = 0;//(initial << 2) >> 8;
			value[1] = 0;//(initial << 2) & 0xff;
					
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
	UInt16 rpm = (UInt16(data[0] << 8) | (data[1] & 0xff)) >> 2;
	UInt8 slope = rpm * 127 / m_Maximum;
			
	if (slope == 0)
		slope = m_DefaultStartPWM;
	
	DebugLog("Fan #%d SLOPE=0x%x RPM=%drpm", m_Offset, slope, rpm);
		
	outb(m_Provider->GetAddress() + ITE_ADDRESS_REGISTER_OFFSET, ITE_SMARTGUARDIAN_START_PWM[m_Offset]);
	outb(m_Provider->GetAddress() + ITE_DATA_REGISTER_OFFSET, slope);
}