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

UInt8 ReadByte(UInt8 reg)
{
	outb(RegisterPort, reg);
	return inb(ValuePort);
}

UInt16 ReadWord(UInt8 reg)
{
	return ((ReadByte(reg) << 8) | ReadByte(reg + 1));
}

void Select(UInt8 logicalDeviceNumber)
{
	outb(RegisterPort, DEVCIE_SELECT_REGISTER);
	outb(ValuePort, logicalDeviceNumber);
}

void UpdateChipName()
{
	switch (Model) 
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

// ITE

void IT87Enter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x01);
	outb(RegisterPort, 0x55);
	outb(RegisterPort, 0x55);
}

void IT87Exit()
{
	outb(RegisterPort, CONFIGURATION_CONTROL_REGISTER);
	outb(ValuePort, 0x02);
}

short IT87ReadByte(UInt8 reg, bool* valid)
{
	outb(Address + ITE_ADDRESS_REGISTER_OFFSET, reg);
	UInt8 value = inb(Address + ITE_DATA_REGISTER_OFFSET);
	
	valid = (bool*)(reg == inb(Address + ITE_DATA_REGISTER_OFFSET));
	
	return value;
}

// WinbondFintek

void WinbondFintekEnter()
{
	outb(RegisterPort, 0x87);
	outb(RegisterPort, 0x87);
}

void WinbondFintekExit()
{
	outb(RegisterPort, 0xAA);      
}

bool WinboundFintekProbe()
{
	// Winbound
	
	WinbondFintekEnter();
	
	UInt8 id = ReadByte(CHIP_ID_REGISTER);
	UInt8 logicalDeviceNumber = 0;
	
	Revision = ReadByte(CHIP_REVISION_REGISTER);
	
	switch (id) 
	{
		case 0x05:
			switch (Revision) 
			{
				case 0x07:
					Model = F71858;
					logicalDeviceNumber = F71858_HARDWARE_MONITOR_LDN;
					break;
				case 0x41:
					Model = F71882;
					logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
					break;              
			} 
			break;
		case 0x06:
			switch (Revision) 
			{             
				case 0x01:
					Model = F71862;
					logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
					break;              
			}
			break;
		case 0x07:
			switch (Revision) 
			{
				case 0x23:
					Model = F71889F;
					logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
					break;              
			} 
			break;
		case 0x08:
			switch (Revision)
			{
				case 0x14:
					Model = F71869;
					logicalDeviceNumber = FINTEK_HARDWARE_MONITOR_LDN;
					break;              
			}
			break;
		case 0x09:
			switch (Revision)
			{
				case 0x09:
					Model = F71889ED;
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
					Model = W83627HF;
					logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
					break;             
			}
			break;
		case 0x82:
			switch (Revision)
			{
				case 0x83:
					Model = W83627THF;
					logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
					break;
			}
			break;
		case 0x85:
			switch (Revision)
			{
				case 0x41:
					Model = W83687THF;
					logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
					break;
			}
			break;
		case 0x88:
			switch (Revision & 0xF0)
			{
				case 0x50:
				case 0x60:
					Model = W83627EHF;
					logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
					break;
			}
			break;
		case 0xA0:
			switch (Revision & 0xF0)
			{
				case 0x20: 
					Model = W83627DHG;
					logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;  
					break;             
			}
			break;
		case 0xA5:
			switch (Revision & 0xF0)
			{
				case 0x10:
					Model = W83667HG;
					logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
					break;
			}
			break;
		case 0xB0:
			switch (Revision & 0xF0)
			{
				case 0x70:
					Model = W83627DHGP;
					logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
					break;             
			}
			break;
		case 0xB3:
			switch (Revision & 0xF0)
			{
				case 0x50:
					Model = W83667HGB;
					logicalDeviceNumber = WINBOND_HARDWARE_MONITOR_LDN;
					break;
			}
			break; 
	}
	
	if (Model == UnknownModel)
	{
		if (id != 0 && id != 0xff)
		{
			WinbondFintekExit();
			
			InfoLog("Found Unknown Winbond / Fintek with ID 0x%x", ((id << 8) | Revision)); 		
		}
	} 
	else
	{
		UpdateChipName();
		
		Select(logicalDeviceNumber);
		
		Address = ReadWord(BASE_ADDRESS_REGISTER);          
		
		//usleep(1);
		
		UInt16 verify = ReadWord(BASE_ADDRESS_REGISTER);
		UInt16 vendorID = ReadWord(FINTEK_VENDOR_ID_REGISTER);
		
		WinbondFintekExit();
		
		if (Address != verify)
		{            
			InfoLog("Chip ID - %s", Name); 
			InfoLog("Chip revision - 0x%x", Revision);
			WarningLog("Address verification failed");
			
			return false;
		}
		
		// some Fintek chips have address register offset 0x05 added already
		if ((Address & 0x07) == 0x05)
			Address &= 0xFFF8;
		
		if (Address < 0x100 || (Address & 0xF007) != 0)
		{            
			InfoLog("Chip ID - %s", Name);
			InfoLog("Chip revision - 0x%x", Revision);
			WarningLog("Invalid address 0x%x", Address);
			
			return false;
		}
		
		switch (Model)
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
					InfoLog("Chip ID - %s", Name);
					InfoLog("Chip revision - 0x%x", Revision);
					WarningLog("Invalid vendor ID 0x%x", vendorID);
					
					return false;
				}
				//hardware.Add(new F718XX(chip, address));
				break;
			default: break;
		}
		
		UpdateChipName();
		
		InfoLog("Found %s", Name);
	}
	
	return true;
}

// SMSC

void SMSCEnter()
{
	outb(RegisterPort, 0x55);
}

void SMSCExit()
{
	outb(RegisterPort, 0xAA);
}

bool SMSCProbe()
{
	SMSCEnter();
	
	UInt16 chipID = ReadWord(CHIP_ID_REGISTER);
	
	switch (chipID) 
	{
		default: Model = UnknownModel; break;
	}
	
	if (Model == UnknownModel)
	{
		if (chipID != 0 && chipID != 0xffff)
		{
			SMSCExit();
			
			UpdateChipName();
			
			InfoLog("Found unknown SMSC with ID 0x%x", chipID);
		}
	} 
	else 
	{
		SMSCExit();
		
		return false;
	}
	
	return true;
}

bool CompareKeys(const char* key1, const char* key2)
{
	return ((key1[0] == key2[0]) && (key1[1] == key2[1]) && (key1[2] == key2[2]) && (key1[3] == key2[3]));
}

static void Update(SMCData node)
{
	switch (Type)
	{
		case IT87x:
			bool* valid;
			 	
			// Heatsink
			if(CompareKeys(node->key, "Th0H"))
			{
				char value = IT87ReadByte(ITE_TEMPERATURE_BASE_REG + 1, valid);
				
				if(valid)
				{
					node->data[0] = value;
					node->data[1] = 0;
				}
			}
			
			// Northbridge
			if(CompareKeys(node->key, "TN0P"))
			{
				char value = IT87ReadByte(ITE_TEMPERATURE_BASE_REG, valid);
				
				if(valid)
				{
					node->data[0] = value;
					node->data[1] = 0;
				}
			}
			
			// FANs
			if(CompareKeys(node->key, "F0Ac") || CompareKeys(node->key, "F1Ac") || CompareKeys(node->key, "F2Ac") || CompareKeys(node->key, "F3Ac") || CompareKeys(node->key, "F4Ac"))
			{
				UInt8 num = node->key[1] - 48;
				
				int value = IT87ReadByte(ITE_FAN_TACHOMETER_REG[num], valid);
				
				if(valid)
				{
					value |= IT87ReadByte(ITE_FAN_TACHOMETER_EXT_REG[num], valid) << 8;
						
					if(valid) 
					{	
						if (value > 0x3f) 
						{
							value = 1.35e6f / (value * 2);
						}
											
						// iStat (mac os?) fix
						value *= 4;
						
						node->data[0] = value >> 8;
						node->data[1] = value & 0xff;
					}
				}
			}
			
			break;
		case Winbound:
			break;
		case Fintek:
			break;
	}
}

#define super IOService
OSDefineMetaClassAndStructors(LPCMonitorPlugin, IOService)

bool LPCMonitorPlugin::init(OSDictionary *properties)
{
	DebugLog("Initialising...");
	
    super::init(properties);
	
	return true;
}

IOService* LPCMonitorPlugin::probe(IOService *provider, SInt32 *score)
{
	DebugLog("Probing...");
	
	if (super::probe(provider, score) != this) return 0;
	
	
	for (int i = 0; i < 2; i++)
	{
        RegisterPort = REGISTER_PORT[i];
        ValuePort = VALUE_PORT[i];
	
		Type = UnknownType;
		Model = UnknownModel;
		
		// ITE
		
		IT87Enter();
		
		UInt16 chipID = ReadWord(CHIP_ID_REGISTER);
		
		switch (chipID)
		{
			case 0x8712: 
				Model = IT8712F; 
				break;
			case 0x8716: 
				Model = IT8716F; 
				break;
			case 0x8718: 
				Model = IT8718F; 
				break;
			case 0x8720: 
				Model = IT8720F; 
				break;
			case 0x8726: 
				Model = IT8726F; 
				break; 
			default: 
				Model = UnknownModel;
				break;
		}
		
		if (Model == UnknownModel)
		{
			if (chipID != 0 && chipID != 0xffff)
			{
				IT87Exit();
				
				Type = IT87x;
				
				InfoLog("Found unknown ITE with ID 0x%x", chipID);
				
				break;
			}
			else 
			{
				return 0;
			}

		} 
		else 
		{
			UpdateChipName();
			
			Select(IT87_ENVIRONMENT_CONTROLLER_LDN);
			
			Address = ReadWord(BASE_ADDRESS_REGISTER);
			
			//Thread.Sleep(1);
			
			UInt16 verify = ReadWord(BASE_ADDRESS_REGISTER);
			
			IT87Exit();
			
			if (Address != verify || Address < 0x100 || (Address & 0xF007) != 0)
			{
				InfoLog("Chip ID - %s", Name);
				WarningLog("Invalid address 0x%x", Address);
				
				return 0;
			}
			
			Type = IT87x;
			
			InfoLog("Found %s", Name);
			
			break;
		}
		
	}
	
	return this;
}

bool LPCMonitorPlugin::start(IOService * provider)
{
	DebugLog("Starting...");
	
	if (!super::start(provider)) return false;
	
	switch (Type)
	{
		case IT87x:
			bool* valid;
			UInt8 vendorId;
			
			vendorId = IT87ReadByte(ITE_VENDOR_ID_REGISTER, valid);
			
			if (!valid || vendorId != ITE_VENDOR_ID)
			{
				WarningLog("Invalid vendor ID 0x%x, must be 0x%x", vendorId, ITE_VENDOR_ID);
				return false;
			}
			
			if ((IT87ReadByte(ITE_CONFIGURATION_REGISTER, valid) & 0x10) == 0)
			{
				WarningLog("Bit 0x10 of the configuration register should always be 1");
				return false;
			}
			
			if (!valid)
			{
				WarningLog("Invalid %s reading", Name);
				return false;
			}
			
			char value[2];

			// Heatsink
			FakeSMCRegisterKey("Th0H", 2, value, &Update);
			
			// Northbridge
			FakeSMCRegisterKey("TN0P", 2, value, &Update);
			
			// Fans
			value[0] = 0x5;
			FakeSMCRegisterKey("FNum", 1, value, NULL);
			
			value[0] = 0xef;
			value[1] = 0xff;
			for (int i=0; i<5; i++) 
			{
				char key[5];
				snprintf(key, 5, "F%dMx", i);
				FakeSMCRegisterKey(key, 2, value, NULL);
			}
			
			for (int i=0; i<5; i++) 
			{
				char key[5];
				
				value[0] = 0x0;
				value[1] = 0xf;
				snprintf(key, 5, "F%dMn", i);
				FakeSMCRegisterKey(key, 2, value, NULL);
				
				value[0] = 0x0;
				value[1] = 0x0;
				snprintf(key, 5, "F%dAc", i);
				FakeSMCRegisterKey(key, 2, value, &Update);
			}
			
			break;
	}
	
	return true;	
}

void LPCMonitorPlugin::stop (IOService* provider)
{
	DebugLog("Stoping...");
	
	super::stop(provider);
}

void LPCMonitorPlugin::free ()
{
	DebugLog("Freeing...");
	
	super::free ();
}