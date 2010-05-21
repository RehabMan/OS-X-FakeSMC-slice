/*
 *
 *  Copyright © 2010 mozodojo. All rights reserved.
 *
 *	This code includes parts of original Open Hardware Monitor code
 *	Copyright © 2010 Michael Möller. All Rights Reserved.
 *
 */

#include <architecture/i386/pio.h>
//#include <unistd.h>

#include "FakeSMCLPCMonitor.h"

static void Update(SMCData node)
{
	/*UInt32 magic = 0;
	
	mp_rendezvous_no_intrs(IntelThermal, &magic);
	
	UInt8 cpun = node->key[2] - 48;
	
	if(CpuCoreiX)
	{
		node->data[0] = TjMaxCoreiX[cpun] - Thermal[cpun];
	}
	else 
	{
		node->data[0] = TjMax - Thermal[cpun];
	}
	
	node->data[1] = 0;*/
}

#define super IOService
OSDefineMetaClassAndStructors(LPCMonitorPlugin, IOService)

UInt8 LPCMonitorPlugin::ReadByte(UInt8 reg)
{
	outb(RegisterPort, reg);
	return inb(ValuePort);
}

UInt16 LPCMonitorPlugin::ReadWord(UInt8 reg)
{
	return ((ReadByte(reg) << 8) | ReadByte(reg + 1));
}

void LPCMonitorPlugin::Select(UInt8 logicalDeviceNumber)
{
	outb(RegisterPort, DEVCIE_SELECT_REGISTER);
	outb(ValuePort, logicalDeviceNumber);
}

void LPCMonitorPlugin::IT87Enter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x01);
	outb(RegisterPort, 0x55);
	outb(RegisterPort, 0x55);
}

void LPCMonitorPlugin::IT87Exit()
{
	outb(RegisterPort, CONFIGURATION_CONTROL_REGISTER);
	outb(ValuePort, 0x02);
}

void LPCMonitorPlugin::WinbondFintekEnter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x87);
}

void LPCMonitorPlugin::WinbondFintekExit()
{
	outb(RegisterPort, 0xAA);      
}

void LPCMonitorPlugin::SMSCEnter()
{
	outb(RegisterPort, 0x55);
}

void LPCMonitorPlugin::SMSCExit()
{
	outb(RegisterPort, 0xAA);
}

void LPCMonitorPlugin::UpdateName()
{
	switch (Chip) 
	{
        case F71858: Name = "Fintek F71858"; break;
        case F71862: Name = "Fintek F71862"; break;
        case F71869: Name = "Fintek F71869"; break;
        case F71882: Name = "Fintek F71882"; break;
        case F71889ED: Name = "Fintek F71889ED"; break;
        case F71889F: Name = "Fintek F71889F"; break;
        case IT8712F: Name = "ITE IT8712F"; break;
        case IT8716F: Name = "ITE IT8716F"; break;
        case IT8718F: Name = "ITE IT8718F"; break;
        case IT8720F: Name = "ITE IT8720F"; break;
        case IT8726F: Name = "ITE IT8726F"; break;
        case W83627DHG: Name = "Winbond W83627DHG"; break;
        case W83627DHGP: Name = "Winbond W83627DHG-P"; break;
        case W83627EHF: Name = "Winbond W83627EHF"; break;
        case W83627HF: Name = "Winbond W83627HF"; break;
        case W83627THF: Name = "Winbond W83627THF"; break;
        case W83667HG: Name = "Winbond W83667HG"; break;
        case W83667HGB: Name = "Winbond W83667HG-B"; break;
        case W83687THF: Name = "Winbond W83687THF"; break;
		default: Name = "Unknown";
    };
}

IOService* LPCMonitorPlugin::probe(IOService *provider, SInt32 *score)
{
	if (super::probe(provider, score) != this) return 0;
	
	for (int i = 0; i < 2; i++)
	{
        RegisterPort = LPCRegisterPort[i];
        ValuePort = LPCValuePort[i];
		
		// Winvound
		
        WinbondFintekEnter();
		
        UInt8 logicalDeviceNumber = 0;

        Chip = Unknown;
		
		UInt8 id = ReadByte(CHIP_ID_REGISTER);
		Revision = ReadByte(CHIP_REVISION_REGISTER);
		
        switch (id) 
		{
			case 0x05:
				switch (Revision) 
				{
					case 0x07:
						Chip = F71858;
						logicalDeviceNumber = F71858_HARDWARE_MONITOR_LDN;
						break;
					case 0x41:
						Chip = F71882;
						logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
						break;              
				} 
				break;
			case 0x06:
				switch (Revision) 
				{             
					case 0x01:
						Chip = F71862;
						logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
						break;              
				}
				break;
			case 0x07:
				switch (Revision) 
				{
					case 0x23:
						Chip = F71889F;
						logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
						break;              
				} 
				break;
			case 0x08:
				switch (Revision)
				{
					case 0x14:
						Chip = F71869;
						logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
						break;              
				}
				break;
			case 0x09:
				switch (Revision)
				{
					case 0x09:
						Chip = F71889ED;
						logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
						break;              
				} 
				break;
			case 0x52:
				switch (Revision) 
				{
					case 0x17:
					case 0x3A:
					case 0x41:
						Chip = W83627HF;
						logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
						break;             
				}
				break;
			case 0x82:
				switch (Revision)
				{
					case 0x83:
						Chip = W83627THF;
						logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
						break;
				}
				break;
			case 0x85:
				switch (Revision)
				{
					case 0x41:
						Chip = W83687THF;
						logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
						break;
				}
				break;
			case 0x88:
				switch (Revision & 0xF0)
				{
					case 0x50:
					case 0x60:
						Chip = W83627EHF;
						logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
						break;
				}
				break;
			case 0xA0:
				switch (Revision & 0xF0)
				{
					case 0x20: 
						Chip = W83627DHG;
						logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;  
						break;             
				}
				break;
			case 0xA5:
				switch (Revision & 0xF0)
				{
					case 0x10:
						Chip = W83667HG;
						logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
						break;
				}
				break;
			case 0xB0:
				switch (Revision & 0xF0)
				{
					case 0x70:
						Chip = W83627DHGP;
						logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
						break;             
				}
				break;
			case 0xB3:
				switch (Revision & 0xF0)
				{
					case 0x50:
						Chip = W83667HGB;
						logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
						break;
				}
				break; 
        }
		
        if (Chip == Unknown)
		{
			if (id != 0 && id != 0xff)
			{
				WinbondFintekExit();
				
				InfoLog("Chip ID: Unknown Winbond / Fintek with ID 0x%x", ((id << 8) | Revision)); 		
			}
        } 
		else
		{
			UpdateName();
			
			Select(logicalDeviceNumber);
			
			Address = ReadWord(BASE_ADDRESS_REGISTER);          
			
			//usleep(1);
			
			UInt16 verify = ReadWord(BASE_ADDRESS_REGISTER);
			UInt16 vendorID = ReadWord(FINTEK_VENDOR_ID_REGISTER);
			
			WinbondFintekExit();
			
			if (Address != verify)
			{            
				InfoLog("Chip ID: %s", Name); 
				InfoLog("Chip revision: 0x%x", Revision);
				WarningLog("Address verification failed");
				
				return NULL;
			}
			
			// some Fintek chips have address register offset 0x05 added already
			if ((Address & 0x07) == 0x05)
				Address &= 0xFFF8;
			
			if (Address < 0x100 || (Address & 0xF007) != 0)
			{            
				InfoLog("Chip ID: %s", Name);
				InfoLog("Chip revision: 0x%x", Revision);
				WarningLog("Invalid address 0x%x", Address);
				
				return NULL;
			}
			
			switch (Chip)
			{
				case W83627DHG:
				case W83627DHGP:
				case W83627EHF:
				case W83627HF:
				case W83627THF:
				case W83667HG:
				case W83667HGB:
				case W83687THF:
					/*W836XX w836XX = new W836XX(chip, revision, address);
					if (w836XX.IsAvailable)
						hardware.Add(w836XX);*/
					break;
				case F71858:
				case F71862:
				case F71869:
				case F71882:
				case F71889ED:
				case F71889F:
					if (vendorID != FINTEK_VENDOR_ID)
					{
						InfoLog("Chip ID: %s", Name);
						InfoLog("Chip revision: 0x%x", Revision);
						WarningLog("Invalid vendor ID 0x%x", vendorID);
						
						return NULL;
					}
					//hardware.Add(new F718XX(chip, address));
					break;
				default: 
					break;
			}
			
			UpdateName();
			
			InfoLog("Chip ID: %s with ID 0x%x", Name, id);
			
			return this;
        }
		
		// IT87x
		
		IT87Enter();
		
        UInt16 chipID = ReadWord(CHIP_ID_REGISTER);
		
        switch (chipID)
		{
			case 0x8712: 
				Chip = IT8712F; 
				break;
			case 0x8716: 
				Chip = IT8716F; 
				break;
			case 0x8718: 
				Chip = IT8718F; 
				break;
			case 0x8720: 
				Chip = IT8720F; 
				break;
			case 0x8726: 
				Chip = IT8726F; 
				break; 
			default: 
				Chip = Unknown; 
				break;
        }
		
        if (Chip == Unknown)
		{
			if (chipID != 0 && chipID != 0xffff)
			{
				IT87Exit();
				
				InfoLog("Chip ID: Unknown ITE with ID 0x%x", chipID);
				
				return this;
			}
        } 
		else 
		{
			UpdateName();
			
			Select(IT87_ENVIRONMENT_CONTROLLER_LDN);
			
			Address = ReadWord(BASE_ADDRESS_REGISTER);
			
			//Thread.Sleep(1);
			
			UInt16 verify = ReadWord(BASE_ADDRESS_REGISTER);
			
			IT87Exit();
			
			if (Address != verify || Address < 0x100 || (Address & 0xF007) != 0)
			{
				InfoLog("Chip ID: %s", Name);
				WarningLog("Invalid address 0x%x", Address);
				
				return NULL;
			}
			
			/*IT87XX it87 = new IT87XX(chip, address);
			 if (it87.IsAvailable)
			 hardware.Add(it87);*/
			
			InfoLog("Chip ID: %s with ID 0x%x", Name, chipID);
			
			return this;
        }
		
		// SMSC
        SMSCEnter();
		
        chipID = ReadWord(CHIP_ID_REGISTER);
		
        switch (chipID) 
		{
			default: Chip = Unknown; break;
        }
		
        if (Chip == Unknown)
		{
			if (chipID != 0 && chipID != 0xffff)
			{
				SMSCExit();

				UpdateName();
				
				InfoLog("Chip ID: Unknown SMSC with ID 0x%x", chipID);
				
				return this;
			}
        } 
		else 
		{
			SMSCExit();

			return NULL;
        }
	}
	
	return NULL;
}

bool LPCMonitorPlugin::start(IOService * provider)
{
	if (!super::start(provider)) return false;
	
	registerService(0);
	
	return true;	
}

bool LPCMonitorPlugin::init(OSDictionary *properties)
{
    super::init(properties);
	
	return true;
}

void LPCMonitorPlugin::stop (IOService* provider)
{
	super::stop(provider);
}

void LPCMonitorPlugin::free ()
{
	super::free ();
}