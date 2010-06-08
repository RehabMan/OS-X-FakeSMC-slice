/*
 *  I2Cline.h
 *  FakeSMCSMBus
 *
 *  Created by Slice on 07.06.10.
 *  Copyright 2010 Slice. All rights reserved.
 *
 */

#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOLocks.h>

#define MAX_NUM_I2C 8
#define MAX_NUM_REGS 8


typedef struct {
	// times
	int t_out;
	int t_low;
	int t_high;
	int t_buf;
	int tsu_dat;
	int thd_dat;
	int tsu_sta;
	int thd_sta;
	int tsu_sto;
	
	// registers
	int n_out_clk_reg;
	int n_out_data_reg;
	UInt32 in_clk_reg;
	UInt32 in_data_reg;
	UInt32 out_clk_reg;
	UInt32 out_data_reg;
	
	// masks
	UInt32 clk_mask;
	UInt32 data_mask;	
	UInt32 clk_sign;
	UInt32 data_sign;
	
} I2Cdefines;

enum {
	PCI_mode, 
	Memory_Mode,
	SystemIO_Mode
};

class I2Cline : public OSObject 
{
    OSDeclareDefaultStructors(I2Cline)    
private:
protected:	
	void		WriteReg(UInt32 reg, UInt32 value);
	UInt32		ReadReg(UInt32 reg);
	void	PutBits(int line, UInt32 clk, UInt32 data);
	bool	GetBits(int line, UInt32 * scl, UInt32 * sda);
	bool	Busy(int line);
	bool	Start(int line);
	void	Stop(int line);
	void	Send9Stop(int line);
	bool	Ack(int line);
	void	SendAck(int line);
	void	SendNack(int line);
	bool	WriteByte(int line, UInt8 byte);
	bool	ReadByte(int line, UInt8 * byte);
public:
	volatile UInt8* mmio_base;
	int				Mode;
	IOPCIDevice	*	I2Cdevice;
	UInt32			addr[5];  // dunno how many
	UInt32			command[20]; 
	I2Cdefines		I2C[MAX_NUM_I2C]; // 8 lines for I2C
	UInt32			oldvalues[MAX_NUM_REGS]; // 8 different regs for its
	virtual bool	WriteData16(int line, UInt8 addr, UInt8 command, UInt16 value);
	virtual bool	ReadData16(int line, UInt8 addr, UInt8 command, UInt16 * value);
};



