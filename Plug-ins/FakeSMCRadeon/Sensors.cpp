/*
 *  Sensors.cpp
 *  FakeSMCRadeon
 *
 *  Created by Slice on 24.07.10.
 *  Copyright 2010 Applelife.ru. All rights reserved.
 *
 */
#include "Sensor.h"

IOReturn R5xxTemperatureSensor::OnKeyRead(const char* key, char* data)
{
	return kIOReturnUnsupported;
}

IOReturn R6xxTemperatureSensor::OnKeyRead(const char* key, char* data)
{
	UInt32 temp, actual_temp = 0;
	for (int i=0; i<1000; i++) {  //attempts to ready
		temp = (m_Provider->read32(CG_THERMAL_STATUS) & ASIC_T_MASK) >> ASIC_T_SHIFT;	
		if ((temp >> 7) & 1)
			actual_temp = 0;
		else {
			actual_temp = (temp >> 1) & 0xff;
			break;
		}
		IOSleep(10);
	}
	data[0] = actual_temp;
	data[1] = 0;
	return kIOReturnSuccess; 
	
}

IOReturn R7xxTemperatureSensor::OnKeyRead(const char* key, char* data)
{
	UInt32 temp, actual_temp = 0;
	for (int i=0; i<1000; i++) {  //attempts to ready
		temp = (m_Provider->read32(CG_MULT_THERMAL_STATUS) & ASIC_TM_MASK) >> ASIC_TM_SHIFT;	
		if ((temp >> 9) & 1)
			actual_temp = 0;
		else {
			actual_temp = (temp >> 1) & 0xff;
			break;
		}
		IOSleep(10);
	}

	data[0] = actual_temp;
	data[1] = 0;
	return kIOReturnSuccess;
}

IOReturn EverTemperatureSensor::OnKeyRead(const char* key, char* data)
{
	UInt32 temp, actual_temp = 0;
	for (int i=0; i<1000; i++) {  //attempts to ready
		temp = (m_Provider->read32(CG_MULT_THERMAL_STATUS) & ASIC_TM_MASK) >> ASIC_TM_SHIFT;	
		if ((temp >> 10) & 1)
			actual_temp = 0;
		else if ((temp >> 9) & 1)
			actual_temp = 255;
		else {
			actual_temp = (temp >> 1) & 0xff;
			break;
		}
		IOSleep(10);
	}
	
	data[0] = actual_temp;
	data[1] = 0;
	return kIOReturnSuccess;
}
