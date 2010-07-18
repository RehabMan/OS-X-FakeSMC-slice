/*
 *  ITESensors.cpp
 *  FakeSMCSuperIO
 *
 *  Created by mozo on 15/06/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "ITESensors.h"

IOReturn ITETemperatureSensor::OnKeyRead(__unused const char* key, char* data) {
	data[0] = m_Provider->ReadTemperature(m_Offset);
	data[1] = 0;
	return kIOReturnSuccess;
}

IOReturn ITETemperatureSensor::OnKeyWrite(__unused const char* key, __unused char* data){return kIOReturnSuccess;}

IOReturn ITEVoltageSensor::OnKeyRead(__unused const char* key, char* data) {
	UInt16 value = fp2e_Encode(m_Provider->ReadVoltage(m_Offset));
	
	data[0] = (value & 0xff00) >> 8;
	data[1] = value & 0x00ff;
	return kIOReturnSuccess;
}

IOReturn ITEVoltageSensor::OnKeyWrite(__unused const char* key, __unused char* data){return kIOReturnSuccess;}

IOReturn ITETachometerSensor::OnKeyRead(__unused const char* key, char* data) {
	UInt16 value = m_Provider->ReadTachometer(m_Offset);
	
	data[0] = (value >> 6) & 0xff;
	data[1] = (value << 2) & 0xff;
	return kIOReturnSuccess;
}

IOReturn ITETachometerSensor::OnKeyWrite(__unused const char* key, __unused char* data){return kIOReturnSuccess;}