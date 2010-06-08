/*
 *  I2Cline.cpp
 *  FakeSMCSMBus
 *
 *  Created by Slice on 07.06.10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include "I2Cline.h"

#define INVID(offset) OSReadLittleInt32((mmio_base), offset)
#define OUTVID(offset,val) OSWriteLittleInt32((mmio_base), offset, val)

UInt32 I2Cline::ReadReg(UInt32 reg)
{
	UInt32 value = 0;
	switch (Mode) {
		case PCI_mode:
			value = I2Cdevice->configRead32(reg);
			break;
		case Memory_Mode:	
			value = INVID(reg);
			break;
		case SystemIO_Mode:	
//			value = inb(reg);
			break;
		default:
			break;
	}
	return value;
}

void I2Cline::WriteReg(UInt32 reg, UInt32 value)
{
	UInt8 r = reg;
	switch (Mode) {
		case PCI_mode:
			I2Cdevice->configWrite32(r, value);
			break;
		case Memory_Mode:	
			OUTVID(reg, value);
			break;
		case SystemIO_Mode:	
//			outb(r, value);
			break;
		default:
			break;
	}	
}


/*
 We need to write simultaneously SCl and DATA because of the same register.
 But these bits may have different polarity
 For example AMD bus
 DATA: 0 - low 1 - high
 SCL:  0 - high 1 - low
 And we need to keep all other bits untouched. Suppose they are in oldvalue
 If there are different registers for CLK and DATA it would be two values
 */
void I2Cline::PutBits(int line, UInt32 clk, UInt32 data)
{
	bool same = (I2C[line].n_out_clk_reg == I2C[line].n_out_data_reg);
//	LOCK();
	//Fill CLK bit
	UInt32 value = oldvalues[I2C[line].n_out_clk_reg];   //index of the reg
	if (I2C[line].clk_sign) {    // sign=1 if high=1  else 0
		if(clk) value |= I2C[line].clk_mask;
			else value &= ~I2C[line].clk_mask;
	}
	else {
		if(!clk) value |= I2C[line].clk_mask;
		else value &= ~I2C[line].clk_mask;
	}
	// if different we may output clk
	if (!same) {
		WriteReg(I2C[line].out_clk_reg, value);
		oldvalues[I2C[line].n_out_clk_reg] = value;
		value = oldvalues[I2C[line].n_out_data_reg];
	}
	//Fill DATA bits
	if (I2C[line].data_sign) {
		if(data) value |= I2C[line].data_mask;
		else value &= ~I2C[line].data_mask;
	}
	else {
		if(!data) value |= I2C[line].data_mask;
		else value &= ~I2C[line].data_mask;
	}
	oldvalues[I2C[line].n_out_data_reg] = value;
	// output DATA and CLK if the same
	WriteReg(I2C[line].out_data_reg, value);
//	UNLOCK();
}

bool  I2Cline::GetBits(int line, UInt32 * scl, UInt32 * sda)
{
	UInt32 clk, data;
	clk = ReadReg(I2C[line].in_clk_reg);
	*scl = ((clk & I2C[line].clk_mask)? I2C[line].clk_sign: (1-I2C[line].clk_sign));
	
	data = ReadReg(I2C[line].in_data_reg);
	*sda = ((data & I2C[line].data_mask)? I2C[line].data_sign: (1-I2C[line].data_sign));
	return true;
}

bool I2Cline::Busy(int line)  //wait for CLK low true=busy (CLK=1)
{
	UInt32 scl, sda;
	PutBits(line, 0, 0);
	IOPause(I2C[line].t_low);
	for (int i=0; i<1000; i++) {
		GetBits(line, &scl, &sda);
		if (!scl) {
			return false;  //ready
		}
		IOPause(I2C[line].t_out); //timeout 25mks*1000 = 25ms
	}
	return true;
}

bool I2Cline::Start(int line)
{
	if (Busy(line)) {
		return false;
	}
	PutBits(line, 1, 1); // CLK=1 - Enable DATA=1
	IOPause(I2C[line].tsu_sta);						// 4,7mks for me
	PutBits(line, 1, 0);				// CLK=1 DATA go 1->0 - start I2C
	IOPause(I2C[line].thd_sta);				// 4mks for me	
	PutBits(line, 0, 0); // release
	return true;
}

void I2Cline::Stop(int line)
{
	PutBits(line, 1, 0); // CLK=1 - Enable DATA=0
	IOPause(I2C[line].tsu_sto);  // 4mks for me
	PutBits(line, 1, 1); // DATA go 0->1 - stop I2C
	IOPause(I2C[line].t_buf);  // 4,7mks for me
	PutBits(line, 0, 0);; // release
}

void I2Cline::Send9Stop(int line)
{
	for (int i=0; i<9; i++) {
		Stop(line);
		IOPause(I2C[line].t_buf);
	}
}

bool I2Cline::Ack(int line)  // acknowledge from slave to master CLK 0->1->0 DATA=0
{
	UInt32 scl, sda;
	PutBits(line, 1, 0);
	IOPause(I2C[line].t_high);
	PutBits(line, 0, 0);
	IOPause(I2C[line].t_low);
	for (int i=0; i<1000; i++) {
		GetBits(line, &scl, &sda);
		if (scl) {
			break;
		}
		IOPause(I2C[line].t_out); //timeout 25mks*1000 = 25ms
	}
	if (Busy(line)) {
		return false;
	}
	GetBits(line, &scl, &sda);
	if (!scl || sda) {
		return false;
	}
	return true;
}

void I2Cline::SendAck(int line)
{
	PutBits(line, 0, 0); // CLK=1 - Enable DATA=0
	IOPause(I2C[line].t_low);  // 4mks for me
	PutBits(line, 1, 0); // DATA go 0->1 - stop I2C
	IOPause(I2C[line].t_high);  // 4,7mks for me
	PutBits(line, 0, 0); // release
}


void I2Cline::SendNack(int line)
{
	PutBits(line, 0, 0); // CLK low, DATA low
	IOPause(I2C[line].t_low);
	PutBits(line, 0, 1); // DATA high
	IOPause(I2C[line].tsu_dat);
	PutBits(line, 1, 1); //CLK high
	IOPause(I2C[line].t_high);
	PutBits(line, 0, 1); // clk low
	IOPause(I2C[line].thd_dat);
}


bool I2Cline::WriteByte(int line, UInt8 byte)
{
	if (Busy(line)) {
		return false;
	}
	
	UInt8 bit;
	// Assume that I2C started SCL=tristate=0
	for (UInt8 mask = 1<<8; mask != 0; mask >>= 1) {
		bit = (byte & mask)? 1:0;
		PutBits(line, 0, bit);
		IOPause(I2C[line].tsu_dat);  // 250ns
		PutBits(line, 1, bit); // CLK 1->0 data valid
		IOPause(I2C[line].t_high);  // time to data transfer 4mks
		PutBits(line, 0, bit); //Keep data while clk ->0
		IOPause(I2C[line].thd_dat);  // 300ns
	}
	return true;
}


bool I2Cline::ReadByte(int line, UInt8 * byte)
{
	if (Busy(line)) {
		return false;
	}
	
//	bool ret;
	UInt32 scl, sda, value;
	
	for (UInt8 mask = 1<<8; mask != 0; mask >>= 1) {
		for (int i=0; i<1000; i++) {
			GetBits(line, &scl, &sda);
			if (scl) {
				break;  //wait for scl=1
			}
			IOPause(I2C[line].t_out); //timeout 25mks*1000 = 25ms
		}
		
		GetBits(line, &scl, &sda); //get data
		if (scl) {
			value |= sda? mask:0;
		} else return false; //timeout waiting for scl high
		if (Busy(line)) {
			return false;  //wait for scl low
		}
		
	}
	*byte = value & 0xff;
	return true;
}

bool I2Cline::WriteData16(int line, UInt8 addr, UInt8 command, UInt16 value)
{
	Send9Stop(line);
	if(!Start(line)) return false;
	if(!WriteByte(line, addr)) return false;
	if(!Ack(line)) return false;
	if(!WriteByte(line, command)) return false;
	if(!Ack(line)) return false;
	
	UInt8 byte = value & 0xff;
	if(!WriteByte(line, byte)) return false;
	if(!Ack(line)) return false;
	byte = value >> 8;
	if(!WriteByte(line, byte)) return false;
	if(!Ack(line)) return false;
	Stop(line);
}

bool I2Cline::ReadData16(int line, UInt8 addr, UInt8 command, UInt16 * value)
{
	UInt8 byte;
	Send9Stop(line);
	if(!Start(line)) return false;
	if(!WriteByte(line, addr)) return false;
	if(!Ack(line)) return false;
	if(!WriteByte(line, command)) return false;
	if(!Ack(line)) return false;

	if(!Start(line)) return false;
	if(!WriteByte(line, addr+1)) return false; //read_addr=write_addr+1
	if(!Ack(line)) return false;
	if(!ReadByte(line, &byte)) return false;
	*value = (UInt16)byte;
	SendAck(line);
	if(!ReadByte(line, &byte)) return false;
	*value |= ((UInt16)byte <<8);
	SendNack(line);
	Stop(line);
}
