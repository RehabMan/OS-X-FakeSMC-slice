/*
 *  NVidia injector
 *
 *  Copyright (C) 2009  Jasmin Fazlic, iNDi
 *
 *  NVidia injector is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  NVidia driver and injector is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NVidia injector.  If not, see <http://www.gnu.org/licenses/>.
 */ 
/*
 * Alternatively you can choose to comply with APSL
 */
 
 
/*
 * DCB-Table parsing is based on software (nouveau driver) originally distributed under following license:
 *
 *
 * Copyright 2005-2006 Erik Waling
 * Copyright 2006 Stephane Marchesin
 * Copyright 2007-2009 Stuart Bennett
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "device_inject.h"
#include "nvidia.h"

#ifndef DEBUG_NVIDIA
#define DEBUG_NVIDIA 0
#endif

#if DEBUG_NVIDIA
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

#define NVIDIA_ROM_SIZE 0x10000
#define PATCH_ROM_SUCCESS 1
#define PATCH_ROM_SUCCESS_HAS_LVDS 2
#define PATCH_ROM_FAILED 0
#define MAX_NUM_DCB_ENTRIES 16

#define TYPE_GROUPED 0xff

extern uint32_t devices_number;

const char *nvidia_compatible_0[]	=	{ "@0,compatible",	"NVDA,NVMac" };
const char *nvidia_compatible_1[]	=	{ "@1,compatible",	"NVDA,NVMac" };
const char *nvidia_device_type_0[]	=	{ "@0,device_type",	"display" };
const char *nvidia_device_type_1[]	=	{ "@1,device_type",	"display" };
const char *nvidia_device_type[]	=	{ "device_type",	"NVDA,Parent" };
const char *nvidia_name_0[]		=	{ "@0,name",		"NVDA,Display-A" };
const char *nvidia_name_1[]		=	{ "@1,name",		"NVDA,Display-B" };
const char *nvidia_slot_name[]		=	{ "AAPL,slot-name",	"Slot-1" };

static uint8_t default_NVCAP[]= {
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a,
	0x00, 0x00, 0x00, 0x00
};

#define NVCAP_LEN ( sizeof(default_NVCAP) / sizeof(uint8_t) )

static struct nv_chipsets_t NVKnownChipsets[] = {
	{ 0x00000000, "Unknown" },
	{ 0x10DE0040, "GeForce 6800 Ultra" },
	{ 0x10DE0041, "GeForce 6800" },
	{ 0x10DE0042, "GeForce 6800 LE" },
	{ 0x10DE0043, "GeForce 6800 XE" },
	{ 0x10DE0044, "GeForce 6800 XT" },
	{ 0x10DE0045, "GeForce 6800 GT" },
	{ 0x10DE0046, "GeForce 6800 GT" },
	{ 0x10DE0047, "GeForce 6800 GS" },
	{ 0x10DE0048, "GeForce 6800 XT" },
	{ 0x10DE004E, "Quadro FX 4000" },
	{ 0x10DE0090, "GeForce 7800 GTX" },
	{ 0x10DE0091, "GeForce 7800 GTX" },
	{ 0x10DE0092, "GeForce 7800 GT" },
	{ 0x10DE0093, "GeForce 7800 GS" },
	{ 0x10DE0095, "GeForce 7800 SLI" },
	{ 0x10DE0098, "GeForce Go 7800" },
	{ 0x10DE0099, "GeForce Go 7800 GTX" },
	{ 0x10DE009D, "Quadro FX 4500" },
	{ 0x10DE00C0, "GeForce 6800 GS" },
	{ 0x10DE00C1, "GeForce 6800" },
	{ 0x10DE00C2, "GeForce 6800 LE" },
	{ 0x10DE00C3, "GeForce 6800 XT" },
	{ 0x10DE00C8, "GeForce Go 6800" },
	{ 0x10DE00C9, "GeForce Go 6800 Ultra" },
	{ 0x10DE00CC, "Quadro FX Go1400" },
	{ 0x10DE00CD, "Quadro FX 3450/4000 SDI" },
	{ 0x10DE00CE, "Quadro FX 1400" },
	{ 0x10DE0140, "GeForce 6600 GT" },
	{ 0x10DE0141, "GeForce 6600" },
	{ 0x10DE0142, "GeForce 6600 LE" },
	{ 0x10DE0143, "GeForce 6600 VE" },
	{ 0x10DE0144, "GeForce Go 6600" },
	{ 0x10DE0145, "GeForce 6610 XL" },
	{ 0x10DE0146, "GeForce Go 6600 TE/6200 TE" },
	{ 0x10DE0147, "GeForce 6700 XL" },
	{ 0x10DE0148, "GeForce Go 6600" },
	{ 0x10DE0149, "GeForce Go 6600 GT" },
	{ 0x10DE014C, "Quadro FX 550" },
	{ 0x10DE014D, "Quadro FX 550" },
	{ 0x10DE014E, "Quadro FX 540" },
	{ 0x10DE014F, "GeForce 6200" },
	{ 0x10DE0160, "GeForce 6500" },
	{ 0x10DE0161, "GeForce 6200 TurboCache(TM)" },
	{ 0x10DE0162, "GeForce 6200SE TurboCache(TM)" },
	{ 0x10DE0163, "GeForce 6200 LE" },
	{ 0x10DE0164, "GeForce Go 6200" },
	{ 0x10DE0165, "Quadro NVS 285" },
	{ 0x10DE0166, "GeForce Go 6400" },
	{ 0x10DE0167, "GeForce Go 6200" },
	{ 0x10DE0168, "GeForce Go 6400" },
	{ 0x10DE0169, "GeForce 6250" },
	{ 0x10DE016A, "GeForce 7100 GS" },
	{ 0x10DE0191, "GeForce 8800 GTX" },
	{ 0x10DE0193, "GeForce 8800 GTS" },
	{ 0x10DE0194, "GeForce 8800 Ultra" },
	{ 0x10DE019D, "Quadro FX 5600" },
	{ 0x10DE019E, "Quadro FX 4600" },
	{ 0x10DE01D1, "GeForce 7300 LE" },
	{ 0x10DE01D3, "GeForce 7300 SE" },
	{ 0x10DE01D6, "GeForce Go 7200" },
	{ 0x10DE01D7, "GeForce Go 7300" },
	{ 0x10DE01D8, "GeForce Go 7400" },
	{ 0x10DE01D9, "GeForce Go 7400 GS" },
	{ 0x10DE01DA, "Quadro NVS 110M" },
	{ 0x10DE01DB, "Quadro NVS 120M" },
	{ 0x10DE01DC, "Quadro FX 350M" },
	{ 0x10DE01DD, "GeForce 7500 LE" },
	{ 0x10DE01DE, "Quadro FX 350" },
	{ 0x10DE01DF, "GeForce 7300 GS" },
	{ 0x10DE0211, "GeForce 6800" },
	{ 0x10DE0212, "GeForce 6800 LE" },
	{ 0x10DE0215, "GeForce 6800 GT" },
	{ 0x10DE0218, "GeForce 6800 XT" },
	{ 0x10DE0221, "GeForce 6200" },
	{ 0x10DE0222, "GeForce 6200 A-LE" },
	{ 0x10DE0240, "GeForce 6150" },
	{ 0x10DE0241, "GeForce 6150 LE" },
	{ 0x10DE0242, "GeForce 6100" },
	{ 0x10DE0244, "GeForce Go 6150" },
	{ 0x10DE0247, "GeForce Go 6100" },
	{ 0x10DE0290, "GeForce 7900 GTX" },
	{ 0x10DE0291, "GeForce 7900 GT" },
	{ 0x10DE0292, "GeForce 7900 GS" },
	{ 0x10DE0298, "GeForce Go 7900 GS" },
	{ 0x10DE0299, "GeForce Go 7900 GTX" },
	{ 0x10DE029A, "Quadro FX 2500M" },
	{ 0x10DE029B, "Quadro FX 1500M" },
	{ 0x10DE029C, "Quadro FX 5500" },
	{ 0x10DE029D, "Quadro FX 3500" },
	{ 0x10DE029E, "Quadro FX 1500" },
	{ 0x10DE029F, "Quadro FX 4500 X2" },
	{ 0x10DE0301, "GeForce FX 5800 Ultra" },
	{ 0x10DE0302, "GeForce FX 5800" },
	{ 0x10DE0308, "Quadro FX 2000" },
	{ 0x10DE0309, "Quadro FX 1000" },
	{ 0x10DE0311, "GeForce FX 5600 Ultra" },
	{ 0x10DE0312, "GeForce FX 5600" },
	{ 0x10DE0314, "GeForce FX 5600XT" },
	{ 0x10DE031A, "GeForce FX Go5600" },
	{ 0x10DE031B, "GeForce FX Go5650" },
	{ 0x10DE031C, "Quadro FX Go700" },
	{ 0x10DE0324, "GeForce FX Go5200" },
	{ 0x10DE0325, "GeForce FX Go5250" },
	{ 0x10DE0326, "GeForce FX 5500" },
	{ 0x10DE0328, "GeForce FX Go5200 32M/64M" },
	{ 0x10DE032A, "Quadro NVS 55/280 PCI" },
	{ 0x10DE032B, "Quadro FX 500/600 PCI" },
	{ 0x10DE032C, "GeForce FX Go53xx Series" },
	{ 0x10DE032D, "GeForce FX Go5100" },
	{ 0x10DE0330, "GeForce FX 5900 Ultra" },
	{ 0x10DE0331, "GeForce FX 5900" },
	{ 0x10DE0332, "GeForce FX 5900XT" },
	{ 0x10DE0333, "GeForce FX 5950 Ultra" },
	{ 0x10DE0334, "GeForce FX 5900ZT" },
	{ 0x10DE0338, "Quadro FX 3000" },
	{ 0x10DE033F, "Quadro FX 700" },
	{ 0x10DE0341, "GeForce FX 5700 Ultra" },
	{ 0x10DE0342, "GeForce FX 5700" },
	{ 0x10DE0343, "GeForce FX 5700LE" },
	{ 0x10DE0344, "GeForce FX 5700VE" },
	{ 0x10DE0347, "GeForce FX Go5700" },
	{ 0x10DE0348, "GeForce FX Go5700" },
	{ 0x10DE034C, "Quadro FX Go1000" },
	{ 0x10DE034E, "Quadro FX 1100" },
	{ 0x10DE0391, "GeForce 7600 GT" },
	{ 0x10DE0392, "GeForce 7600 GS" },
	{ 0x10DE0393, "GeForce 7300 GT" },
	{ 0x10DE0394, "GeForce 7600 LE" },
	{ 0x10DE0395, "GeForce 7300 GT" },
	{ 0x10DE0397, "GeForce Go 7700" },
	{ 0x10DE0398, "GeForce Go 7600" },
	{ 0x10DE0399, "GeForce Go 7600 GT"},
	{ 0x10DE039A, "Quadro NVS 300M" },
	{ 0x10DE039B, "GeForce Go 7900 SE" },
	{ 0x10DE039C, "Quadro FX 550M" },
	{ 0x10DE039E, "Quadro FX 560" },
	{ 0x10DE0400, "GeForce 8600 GTS" },
	{ 0x10DE0401, "GeForce 8600 GT" },
	{ 0x10DE0402, "GeForce 8600 GT" },
	{ 0x10DE0403, "GeForce 8600 GS" },
	{ 0x10DE0404, "GeForce 8400 GS" },
	{ 0x10DE0405, "GeForce 9500M GS" },
	{ 0x10DE0407, "GeForce 8600M GT" },
	{ 0x10DE0408, "GeForce 9650M GS" },
	{ 0x10DE0409, "GeForce 8700M GT" },
	{ 0x10DE040A, "Quadro FX 370" },
	{ 0x10DE040B, "Quadro NVS 320M" },
	{ 0x10DE040C, "Quadro FX 570M" },
	{ 0x10DE040D, "Quadro FX 1600M" },
	{ 0x10DE040E, "Quadro FX 570" },
	{ 0x10DE040F, "Quadro FX 1700" },
	{ 0x10DE0420, "GeForce 8400 SE" },
	{ 0x10DE0421, "GeForce 8500 GT" },
	{ 0x10DE0422, "GeForce 8400 GS" },
	{ 0x10DE0423, "GeForce 8300 GS" },
	{ 0x10DE0424, "GeForce 8400 GS" },
	{ 0x10DE0425, "GeForce 8600M GS" },
	{ 0x10DE0426, "GeForce 8400M GT" },
	{ 0x10DE0427, "GeForce 8400M GS" },
	{ 0x10DE0428, "GeForce 8400M G" },
	{ 0x10DE0429, "Quadro NVS 140M" },
	{ 0x10DE042A, "Quadro NVS 130M" },
	{ 0x10DE042B, "Quadro NVS 135M" },
	{ 0x10DE042C, "GeForce 9400 GT" },
	{ 0x10DE042D, "Quadro FX 360M" },
	{ 0x10DE042E, "GeForce 9300M G" },
	{ 0x10DE042F, "Quadro NVS 290" },
	{ 0x10DE05E0, "GeForce GTX 295" },
	{ 0x10DE05E1, "GeForce GTX 280" },
	{ 0x10DE05E2, "GeForce GTX 260" },
	{ 0x10DE05E3, "GeForce GTX 285" },
	{ 0x10DE05E6, "GeForce GTX 275" },
	{ 0x10DE05EA, "GeForce GTX 260" },
	{ 0x10DE05EB, "GeForce GTX 295" },
	{ 0x10DE05F9, "Quadro CX" },
	{ 0x10DE05FD, "Quadro FX 5800" },
	{ 0x10DE05FE, "Quadro FX 4800" },
  { 0x10DE0600, "GeForce 8800 GTS 512" },
	{ 0x10DE0602, "GeForce 8800 GT" },
	{ 0x10DE0604, "GeForce 9800 GX2" },
	{ 0x10DE0605, "GeForce 9800 GT" },
	{ 0x10DE0606, "GeForce 8800 GS" },
	{ 0x10DE0607, "GeForce GTS 240" },
	{ 0x10DE0608, "GeForce 9800M GTX" },
	{ 0x10DE0609, "GeForce 8800M GTS" },
	{ 0x10DE060A, "GeForce GTX 280M" },
	{ 0x10DE060B, "GeForce 9800M GT" },
	{ 0x10DE060C, "GeForce 8800M GTX" },
	{ 0x10DE060D, "GeForce 8800 GS" },
	{ 0x10DE0610, "GeForce 9600 GSO" },
	{ 0x10DE0611, "GeForce 8800 GT" },
	{ 0x10DE0612, "GeForce 9800 GTX" },
	{ 0x10DE0613, "GeForce 9800 GTX+" },
	{ 0x10DE0614, "GeForce 9800 GT" },
	{ 0x10DE0615, "GeForce GTS 250" },
	{ 0x10DE0617, "GeForce 9800M GTX" },
	{ 0x10DE0618, "GeForce GTX 260M" },	
	{ 0x10DE061A, "Quadro FX 3700" },
	{ 0x10DE061C, "Quadro FX 3600M" },
	{ 0x10DE061D, "Quadro FX 2800M" },
	{ 0x10DE061F, "Quadro FX 3800M" },
	{ 0x10DE0622, "GeForce 9600 GT" },
	{ 0x10DE0623, "GeForce 9600 GS" },
	{ 0x10DE0625, "GeForce 9600 GSO 512"},
	{ 0x10DE0626, "GeForce GT 130" },
	{ 0x10DE0627, "GeForce GT 140" },
	{ 0x10DE0628, "GeForce 9800M GTS" },
	{ 0x10DE062A, "GeForce 9700M GTS" },
	{ 0x10DE062C, "GeForce 9800M GTS" },
	{ 0x10DE0640, "GeForce 9500 GT" },
	{ 0x10DE0641, "GeForce 9400 GT" },
	{ 0x10DE0642, "GeForce 8400 GS" },
	{ 0x10DE0643, "GeForce 9500 GT" },
	{ 0x10DE0644, "GeForce 9500 GS" },
	{ 0x10DE0645, "GeForce 9500 GS" },
	{ 0x10DE0646, "GeForce GT 120" },
	{ 0x10DE0647, "GeForce 9600M GT" },
	{ 0x10DE0648, "GeForce 9600M GS" },
	{ 0x10DE0649, "GeForce 9600M GT" },
	{ 0x10DE064A, "GeForce 9700M GT" },
	{ 0x10DE064B, "GeForce 9500M G" },
	{ 0x10DE064C, "GeForce 9650M GT" },
	{ 0x10DE0652, "GeForce GT 130M" },
	{ 0x10DE0658, "Quadro FX 380" },
	{ 0x10DE0659, "Quadro FX 580" },
	{ 0x10DE065A, "Quadro FX 1700M" },
	{ 0x10DE065B, "GeForce 9400 GT" },
	{ 0x10DE065C, "Quadro FX 770M" },
	{ 0x10DE06E0, "GeForce 9300 GE" },
	{ 0x10DE06E1, "GeForce 9300 GS" },
	{ 0x10DE06E4, "GeForce 8400 GS" },
	{ 0x10DE06E5, "GeForce 9300M GS" },
	{ 0x10DE06E8, "GeForce 9200M GS" },
	{ 0x10DE06E9, "GeForce 9300M GS" },
	{ 0x10DE06EA, "Quadro NVS 150M" },
	{ 0x10DE06EB, "Quadro NVS 160M" },
	{ 0x10DE06EC, "GeForce G 105M" },
	{ 0x10DE06EF, "GeForce G 103M" },
	{ 0x10DE06F8, "Quadro NVS 420" },
	{ 0x10DE06F9, "Quadro FX 370 LP" },
	{ 0x10DE06FA, "Quadro NVS 450" },
	{ 0x10DE06FD, "Quadro NVS 295" },
	{ 0x10DE087D, "ION 9400M" },
	{ 0x10DE087E, "ION LE" },
	{ 0x10DE0A20, "GeForce GT220" },
	{ 0x10DE0A23, "GeForce 210" },
	{ 0x10DE0A28, "GeForce GT 230M" },
	{ 0x10DE0A2A, "GeForce GT 230M" },
	{ 0x10DE0A34, "GeForce GT 240M" },
	{ 0x10DE0A60, "GeForce G210" },
	{ 0x10DE0A62, "GeForce 205" },
	{ 0x10DE0A63, "GeForce 310" },
	{ 0x10DE0A65, "GeForce 210" },
	{ 0x10DE0A66, "GeForce 310" },
	{ 0x10DE0A74, "GeForce G210M" },
	{ 0x10DE0A78, "Quadro FX 380 LP" },
	{ 0x10DE0CA3, "GeForce GT 240" },
	{ 0x10DE0CA8, "GeForce GTS 260M" },
	{ 0x10DE0CA9, "GeForce GTS 250M" },
	{ 0x10DE0CA3, "GeForce GT240" },
	{ 0x10DE06C0, "GeForce GTX 480" },
	{ 0x10DE06CD, "GeForce GTX 470" },
	{ 0x10DE06C4, "GeForce GTX 465" },
	{ 0x10DE06CA, "GeForce GTX 480M" },
	{ 0x10DE0E22, "GeForce GTX 460" },
	{ 0x10DE0E24, "GeForce GTX 460" },
	{ 0x10DE06D1, "Tesla C2050" },	// TODO: sub-device id: 0x0771
	{ 0x10DE06D1, "Tesla C2070" },	// TODO: sub-device id: 0x0772
	{ 0x10DE06DE, "Tesla M2050" },	// TODO: sub-device id: 0x0846
	{ 0x10DE06DE, "Tesla M2070" }	// TODO: sub-device id: ?
};

static uint16_t swap16(uint16_t x)
{
	return (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8));
}

static uint16_t read16(uint8_t *ptr, uint16_t offset)
{
	uint8_t ret[2];
	ret[0] = ptr[offset+1];
	ret[1] = ptr[offset];
	return *((uint16_t*)&ret);
}

#if 0
static uint32_t swap32(uint32_t x)
{
	return ((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8 ) | ((x & 0x00FF0000) >> 8 ) | ((x & 0xFF000000) >> 24);
}

static uint8_t  read8(uint8_t *ptr, uint16_t offset)
{ 
	return ptr[offset];
}

static uint32_t read32(uint8_t *ptr, uint16_t offset)
{
	uint8_t ret[4];
	ret[0] = ptr[offset+3];
	ret[1] = ptr[offset+2];
	ret[2] = ptr[offset+1];
	ret[3] = ptr[offset];
	return *((uint32_t*)&ret);
}
#endif

static int patch_nvidia_rom(uint8_t *rom)
{
	if (!rom || (rom[0] != 0x55 && rom[1] != 0xaa)) {
		printf("False ROM signature: 0x%02x%02x\n", rom[0], rom[1]);
		return PATCH_ROM_FAILED;
	}
	
	uint16_t dcbptr = swap16(read16(rom, 0x36));
	if(!dcbptr) {
		printf("no dcb table found\n");
		return PATCH_ROM_FAILED;
	}/* else
	 printf("dcb table at offset 0x%04x\n", dcbptr);
	 */
	uint8_t *dcbtable = &rom[dcbptr];
	uint8_t dcbtable_version = dcbtable[0];
	uint8_t headerlength = 0;
	uint8_t recordlength = 0;
	uint8_t numentries = 0;
	
	if(dcbtable_version >= 0x20) {
		uint32_t sig;
		
		if(dcbtable_version >= 0x30) {
			headerlength = dcbtable[1];
			numentries = dcbtable[2];
			recordlength = dcbtable[3];
			sig = *(uint32_t *)&dcbtable[6];
		} else {
			sig = *(uint32_t *)&dcbtable[4];
			headerlength = 8;
		}
		if (sig != 0x4edcbdcb) {
			printf("bad display config block signature (0x%8x)\n", sig);
			return PATCH_ROM_FAILED;
		}
	} else if (dcbtable_version >= 0x14) { /* some NV15/16, and NV11+ */
		char sig[8] = { 0 };
		
		strncpy(sig, (char *)&dcbtable[-7], 7);
		recordlength = 10;
		if (strcmp(sig, "DEV_REC")) {
			printf("Bad Display Configuration Block signature (%s)\n", sig);
			return PATCH_ROM_FAILED;
		}
	} else {
		return PATCH_ROM_FAILED;
	}
	
	if(numentries >= MAX_NUM_DCB_ENTRIES)
		numentries = MAX_NUM_DCB_ENTRIES;
	
	uint8_t num_outputs = 0, i=0;
	struct dcbentry {
		uint8_t type;
		uint8_t index;
		uint8_t *heads;
	} entries[numentries];
	
	for (i = 0; i < numentries; i++) {
		uint32_t connection;
		connection = *(uint32_t *)&dcbtable[headerlength + recordlength * i];
		/* Should we allow discontinuous DCBs? Certainly DCB I2C tables can be discontinuous */
		if ((connection & 0x0000000f) == 0x0000000f) /* end of records */ 
			continue;
		if (connection == 0x00000000) /* seen on an NV11 with DCB v1.5 */ 
			continue;
		if ((connection & 0xf) == 0x6) /* we skip type 6 as it doesnt appear on macbook nvcaps */
			continue;
		
		entries[num_outputs].type = connection & 0xf;
		entries[num_outputs].index = num_outputs;
		entries[num_outputs++].heads = (uint8_t*)&(dcbtable[(headerlength + recordlength * i) + 1]);

	}
	
	int has_lvds = false;
	uint8_t channel1 = 0, channel2 = 0;
	
	for(i=0; i<num_outputs; i++) {
		if(entries[i].type == 3) {
			has_lvds = true;
			//printf("found LVDS\n");
			channel1 |= ( 0x1 << entries[i].index);
			entries[i].type = TYPE_GROUPED;
		}
	}
	// if we have a LVDS output, we group the rest to the second channel
	if(has_lvds) {
		for(i=0; i<num_outputs; i++) {
			if(entries[i].type == TYPE_GROUPED)
				continue;
			channel2 |= ( 0x1 << entries[i].index);
			entries[i].type = TYPE_GROUPED;
		}
	} else {
		//
		int x;
		// we loop twice as we need to generate two channels
		for(x=0; x<=1; x++) {
			for(i=0; i<num_outputs; i++) {
				if(entries[i].type == TYPE_GROUPED)
					continue;
				// if type is TMDS, the prior output is ANALOG
				// we always group ANALOG and TMDS
				// if there is a TV output after TMDS, we group it to that channel as well
				if(i && entries[i].type == 0x2) {
					switch (x) {
						case 0:
							//printf("group channel 1\n");
							channel1 |= ( 0x1 << entries[i].index);
							entries[i].type = TYPE_GROUPED;
							if((entries[i-1].type == 0x0)) {
								channel1 |= ( 0x1 << entries[i-1].index);
								entries[i-1].type = TYPE_GROUPED;
							}
							// group TV as well if there is one
							if( ((i+1) < num_outputs) && (entries[i+1].type == 0x1) ) {
								//	printf("group tv1\n");
								channel1 |= ( 0x1 << entries[i+1].index);
								entries[i+1].type = TYPE_GROUPED;
							}
							break;
						case 1:
							//printf("group channel 2 : %d\n", i);
							channel2 |= ( 0x1 << entries[i].index);
							entries[i].type = TYPE_GROUPED;
							if((entries[i-1].type == 0x0)) {
								channel2 |= ( 0x1 << entries[i-1].index);
								entries[i-1].type = TYPE_GROUPED;
							}
							// group TV as well if there is one
							if( ((i+1) < num_outputs) && (entries[i+1].type == 0x1) ) {
								//	printf("group tv2\n");
								channel2 |= ( 0x1 << entries[i+1].index);
								entries[i+1].type = TYPE_GROUPED;
							}
							break;
							
					}
					break;
				}
			}
		}
	}
	
	// if we have left ungrouped outputs merge them to the empty channel
	uint8_t *togroup;// = (channel1 ? (channel2 ? NULL : &channel2) : &channel1);
	togroup = &channel2;
	for(i=0; i<num_outputs;i++)
		if(entries[i].type != TYPE_GROUPED) {
			//printf("%d not grouped\n", i);
			if(togroup)
				*togroup |= ( 0x1 << entries[i].index);
			entries[i].type = TYPE_GROUPED;
		}
	
	if(channel1 > channel2) {
		uint8_t buff = channel1;
		channel1 = channel2;
		channel2 = buff;
	}
	
	default_NVCAP[6] = channel1;
	default_NVCAP[8] = channel2;
	
	// patching HEADS
	for(i=0; i<num_outputs;i++) {
		if(channel1 & (1 << i))
			*entries[i].heads = 1;
		else if(channel2 & (1 << i))
			*entries[i].heads = 2;
	}
	
	return (has_lvds ? PATCH_ROM_SUCCESS_HAS_LVDS : PATCH_ROM_SUCCESS);
}

static char *get_nvidia_model(uint32_t id) {
	int	i;

	for (i=1; i< (sizeof(NVKnownChipsets) / sizeof(NVKnownChipsets[0])); i++) {
		if (NVKnownChipsets[i].device == id) {
			return NVKnownChipsets[i].name;
		}
	}
	return NVKnownChipsets[0].name;
}

static uint32_t load_nvidia_bios_file(const char *filename, uint8_t *buf, int bufsize)
{
	int	fd;
	int	size;

	if ((fd = open_bvdev("bt(0,0)", filename, 0)) < 0) {
		return 0;
	}
	size = file_size(fd);
	if (size > bufsize) {
		printf("Filesize of %s is bigger than expected! Truncating to 0x%x Bytes!\n", filename, bufsize);
		size = bufsize;
	}
	size = read(fd, (char *)buf, size);
	close(fd);
	return size > 0 ? size : 0;
}

static int devprop_add_nvidia_template(struct DevPropDevice *device)
{
	char	tmp[16]; 

	if(!device)
		return 0;

	if(!DP_ADD_TEMP_VAL(device, nvidia_compatible_0))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, nvidia_device_type_0))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, nvidia_name_0))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, nvidia_compatible_1))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, nvidia_device_type_1))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, nvidia_name_1))
		return 0;
	if(!DP_ADD_TEMP_VAL(device, nvidia_device_type))
		return 0;
	// Rek : Dont use sprintf return, it does not WORK !! our custom sprintf() always return 0!
	// len = sprintf(tmp, "Slot-%x", devices_number);
	sprintf(tmp, "Slot-%x",devices_number);
	devprop_add_value(device, "AAPL,slot-name", (uint8_t *) tmp, strlen(tmp));
	devices_number++;

	return 1;
}

int hex2bin(const char *hex, uint8_t *bin, int len)
{
	char	*p;
	int	i;
	char	buf[3];
 
	if (hex == NULL || bin == NULL || len <= 0 || strlen(hex) != len * 2) {
		printf("[ERROR] bin2hex input error\n");
		return -1;
	}
 
	buf[2] = '\0';
	p = (char *) hex;
	for (i=0; i<len; i++) {
		if (p[0] == '\0' || p[1] == '\0' || !isxdigit(p[0]) || !isxdigit(p[1])) {
			printf("[ERROR] bin2hex '%s' syntax error\n", hex);
			return -2;
		}
		buf[0] = *p++;
		buf[1] = *p++;
		bin[i] = (unsigned char) strtoul(buf, NULL, 16);
	}
	return 0;
}

bool setup_nvidia_devprop(pci_dt_t *nvda_dev)
{
	struct DevPropDevice		*device;
	char				*devicepath;
	struct pci_rom_pci_header_t	*rom_pci_header;	
	volatile uint8_t		*regs;
	uint8_t				*rom;
	uint8_t				*nvRom;
	uint32_t			videoRam;
	uint32_t			nvBiosOveride;
	uint32_t			bar[7];
	uint32_t			boot_display;
	int				nvPatch;
	int				len;
	char				biosVersion[32];
	char				nvFilename[32];
	char				kNVCAP[12];
	char				*model;
	const char			*value;
	bool				doit;

	devicepath = get_pci_dev_path(nvda_dev);
	bar[0] = pci_config_read32(nvda_dev->dev.addr, 0x10 );
	regs = (uint8_t *) (bar[0] & ~0x0f);

	// Amount of VRAM in kilobytes
	videoRam = (REG32(0x10020c) & 0xfff00000) >> 10;
	model = get_nvidia_model((nvda_dev->vendor_id << 16) | nvda_dev->device_id);

	// FIXME: dirty fermi hack
	if((nvda_dev->device_id & 0xFFE0) == 0x06C0 ||
	   (nvda_dev->device_id & 0xFFE0) == 0x0E20) {
		switch (nvda_dev->device_id) {
			case 0x06C0: videoRam = 1572864; break; // gtx 480
			case 0x06CD: videoRam = 1310720; break; // gtx 470
			case 0x06C4: videoRam = 1048576; break; // gtx 465
			case 0x06CA: videoRam = 2097152; break; // gtx 480m
			case 0x0E22: videoRam = 1048576; break; // gtx 460
			case 0x0E24: videoRam = 1048576; break; // gtx 460
			case 0x06D1: videoRam = 3145728; break; // tesla c2050/c2070
			case 0x06DE: videoRam = 3145728; break; // tesla m2050/m2070
			default: break;
		}
	}
	
	verbose("nVidia %s %dMB NV%02x [%04x:%04x] :: %s\n",  
		model, (videoRam / 1024),
		(REG32(0) >> 20) & 0x1ff, nvda_dev->vendor_id, nvda_dev->device_id,
		devicepath);

	rom = malloc(NVIDIA_ROM_SIZE);
	sprintf(nvFilename, "/Extra/%04x_%04x.rom", (uint16_t)nvda_dev->vendor_id, (uint16_t)nvda_dev->device_id);
	if (getBoolForKey(kUseNvidiaROM, &doit, &bootInfo->bootConfig) && doit) {
		verbose("Looking for nvidia video bios file %s\n", nvFilename);
		nvBiosOveride = load_nvidia_bios_file(nvFilename, rom, NVIDIA_ROM_SIZE);
		if (nvBiosOveride > 0) {
			verbose("Using nVidia Video BIOS File %s (%d Bytes)\n", nvFilename, nvBiosOveride);
			DBG("%s Signature 0x%02x%02x %d bytes\n", nvFilename, rom[0], rom[1], nvBiosOveride);
		} else {
			printf("ERROR: unable to open nVidia Video BIOS File %s\n", nvFilename);
			return false;
		}
	} else {
		// Otherwise read bios from card
		nvBiosOveride = 0;

		// TODO: we should really check for the signature before copying the rom, i think.

		// PRAMIN first
		nvRom = (uint8_t*)&regs[NV_PRAMIN_OFFSET];
		bcopy((uint32_t *)nvRom, rom, NVIDIA_ROM_SIZE);
		
		// Valid Signature ?
		if (rom[0] != 0x55 && rom[1] != 0xaa) {
			// PROM next
			// Enable PROM access
			(REG32(NV_PBUS_PCI_NV_20)) = NV_PBUS_PCI_NV_20_ROM_SHADOW_DISABLED;

			nvRom = (uint8_t*)&regs[NV_PROM_OFFSET];
			bcopy((uint8_t *)nvRom, rom, NVIDIA_ROM_SIZE);
			
			// disable PROM access
			(REG32(NV_PBUS_PCI_NV_20)) = NV_PBUS_PCI_NV_20_ROM_SHADOW_ENABLED;	

			// Valid Signature ?
			if (rom[0] != 0x55 && rom[1] != 0xaa) {
				// 0xC0000 last
				bcopy((char *)0xc0000, rom, NVIDIA_ROM_SIZE);
				
				// Valid Signature ?
				if (rom[0] != 0x55 && rom[1] != 0xaa) {
					printf("ERROR: Unable to locate nVidia Video BIOS\n");
					return false;
				} else {
					DBG("ROM Address 0x%x Signature 0x%02x%02x\n", nvRom, rom[0], rom[1]);
				}
			} else {
				DBG("PROM Address 0x%x Signature 0x%02x%02x\n", nvRom, rom[0], rom[1]);
			}
		} else {
			DBG("PRAM Address 0x%x Signature 0x%02x%02x\n", nvRom, rom[0], rom[1]);
		}
	}

	if ((nvPatch = patch_nvidia_rom(rom)) == PATCH_ROM_FAILED) {
		printf("ERROR: nVidia ROM Patching Failed!\n");
		return false;
	}

	rom_pci_header = (struct pci_rom_pci_header_t*)(rom + *(uint16_t *)&rom[24]);

	// check for 'PCIR' sig
	if (rom_pci_header->signature == 0x50434952) {
		if (rom_pci_header->device != nvda_dev->device_id) {
			// Get Model from the OpROM
			model = get_nvidia_model((rom_pci_header->vendor << 16) | rom_pci_header->device);
		} else {
			printf("nVidia incorrect PCI ROM signature: 0x%x\n", rom_pci_header->signature);
		}
	}

	if (!string) {
		string = devprop_create_string();
	}
	device = devprop_add_device(string, devicepath);

	/* FIXME: for primary graphics card only */
	boot_display = 1;
	devprop_add_value(device, "@0,AAPL,boot-display", (uint8_t*)&boot_display, 4);

	if(nvPatch == PATCH_ROM_SUCCESS_HAS_LVDS) {
		uint8_t built_in = 0x01;
		devprop_add_value(device, "@0,built-in", &built_in, 1);
	}

	videoRam *= 1024;
	sprintf(biosVersion, "xx.xx.xx - %s", (nvBiosOveride > 0) ? nvFilename : "internal");

	sprintf(kNVCAP, "NVCAP_%04x", nvda_dev->device_id);
	if (getValueForKey(kNVCAP, &value, &len, &bootInfo->bootConfig) && len == NVCAP_LEN * 2) {
		uint8_t	new_NVCAP[NVCAP_LEN];
 
		if (hex2bin(value, new_NVCAP, NVCAP_LEN) == 0) {
			verbose("Using user supplied NVCAP for %s :: %s\n", model, devicepath);
			memcpy(default_NVCAP, new_NVCAP, NVCAP_LEN);
		}
	}

 #if DEBUG_NVCAP
        printf("NVCAP: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x\n",
		default_NVCAP[0], default_NVCAP[1], default_NVCAP[2], default_NVCAP[3],
		default_NVCAP[4], default_NVCAP[5], default_NVCAP[6], default_NVCAP[7],
		default_NVCAP[8], default_NVCAP[9], default_NVCAP[10], default_NVCAP[11],
		default_NVCAP[12], default_NVCAP[13], default_NVCAP[14], default_NVCAP[15],
		default_NVCAP[16], default_NVCAP[17], default_NVCAP[18], default_NVCAP[19]);
#endif
 
	
	devprop_add_nvidia_template(device);
	devprop_add_value(device, "NVCAP", default_NVCAP, NVCAP_LEN);
	devprop_add_value(device, "VRAM,totalsize", (uint8_t*)&videoRam, 4);
	devprop_add_value(device, "model", (uint8_t*)model, strlen(model) + 1);
	devprop_add_value(device, "rom-revision", (uint8_t*)biosVersion, strlen(biosVersion) + 1);
	if (getBoolForKey(kVBIOS, &doit, &bootInfo->bootConfig) && doit) {
		devprop_add_value(device, "vbios", rom, (nvBiosOveride > 0) ? nvBiosOveride : (rom[2] * 512));
	}

	stringdata = malloc(sizeof(uint8_t) * string->length);
	memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
	stringlength = string->length;

	return true;
}
