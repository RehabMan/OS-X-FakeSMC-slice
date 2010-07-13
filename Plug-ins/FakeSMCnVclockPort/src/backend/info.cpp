/* NVClock 0.8 - Linux overclocker for NVIDIA cards
 *
 * site: http://nvclock.sourceforge.net
 *
 * Copyright(C) 2001-2007 Roderick Colenbrander
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include "backend.h"
#include "nvclock.h"

/* This list isn't used much for the speed ranges anymore */
/* Mainly mobile gpu speeds are missing */
const static struct pci_ids ids[] =
{
	{ 0x20, "nVidia Riva TnT", DESKTOP },
	{ 0x28, "nVidia Riva TnT 2 Pro", DESKTOP },
	{ 0x2a, "nVidia Riva TnT 2", DESKTOP },
	{ 0x2b, "nVidia Riva TnT 2", DESKTOP },
	{ 0xa0, "nVidia Riva TnT 2 Aladdin (Integrated)", DESKTOP },
	{ 0x2c, "nVidia Riva TnT 2 VANTA", DESKTOP },
	{ 0x2d, "nVidia Riva TnT 2 M64", DESKTOP },
	{ 0x2e, "nVidia Riva TnT 2 VANTA", DESKTOP },
	{ 0x2f, "nVidia Riva TnT 2 VANTA", DESKTOP },
	{ 0x29, "nVidia Riva TnT 2 Ultra", DESKTOP },
	{ 0x100, "nVidia Geforce 256 SDR", DESKTOP },
	{ 0x101, "nVidia Geforce 256 DDR", DESKTOP },
	{ 0x103, "nVidia Quadro", DESKTOP },
	{ 0x110, "nVidia Geforce 2 MX/MX400", DESKTOP },
	{ 0x111, "nVidia Geforce 2 MX 100/200", DESKTOP },
	{ 0x112, "nVidia Geforce 2 GO", MOBILE },
	{ 0x113, "nVidia Quadro 2 MXR/EX/GO", MOBILE },
	{ 0x1a0, "nVidia Geforce 2 MX integrated", NFORCE },
	{ 0x150, "nVidia Geforce 2 GTS/PRO", DESKTOP },
	{ 0x151, "nVidia Geforce 2 Ti", DESKTOP },
	{ 0x152, "nVidia Geforce 2 Ultra", DESKTOP },
	{ 0x153, "nVidia Quadro 2 Pro", DESKTOP },
	{ 0x170, "nVidia Geforce 4 MX460", DESKTOP },
	{ 0x171, "nVidia Geforce 4 MX440", DESKTOP },
	{ 0x172, "nVidia Geforce 4 MX420", DESKTOP },
	{ 0x173, "nVidia Geforce 4 MX440 SE", DESKTOP },
	{ 0x174, "nVidia Geforce 4 440 Go", MOBILE },
	{ 0x175, "nVidia Geforce 4 420 Go", MOBILE },
	{ 0x176, "nVidia Geforce 4 420 Go 32M", MOBILE },
	{ 0x177, "nVidia Geforce 4 460 Go", MOBILE },
	{ 0x178, "nVidia Quadro 4 550 XGL", DESKTOP },
	{ 0x179, "nVidia Geforce 4 440 Go 64M", MOBILE },
	{ 0x17a, "nVidia Quadro 4 200/400 NVS", DESKTOP },
	{ 0x17b, "nVidia Quadro 4 550 XGL", DESKTOP },
	{ 0x17c, "nVidia Quadro 4 500 GoGL", MOBILE },
	{ 0x17d, "nVidia Geforce 410 Go", MOBILE },
	{ 0x180, "nVidia Geforce 4 MX440 8X", DESKTOP },
	{ 0x181, "nVidia Geforce 4 MX440 8X", DESKTOP },
	{ 0x182, "nVidia Geforce 4 MX440SE 8X", DESKTOP },
	{ 0x185, "nVidia Geforce 4 MX4000", DESKTOP },
	{ 0x183, "nVidia Geforce 4 MX420 8X", DESKTOP },
	{ 0x186, "nVidia Geforce 4 Go", MOBILE },
	{ 0x187, "nVidia Geforce 4 488 Go", MOBILE },
	{ 0x188, "nVidia Quadro 4 580 XGL", DESKTOP },
	{ 0x18a, "nVidia Quadro 4 280 NVS", DESKTOP },
	{ 0x18b, "nVidia Quadro 4 380 XGL", DESKTOP },
	{ 0x18c, "nVidia Quadro NVS 50 PCI", DESKTOP },
	{ 0x18d, "nVidia Geforce 4 448 Go", MOBILE },
	{ 0x1f0, "nVidia Geforce 4 MX integrated", NFORCE },
	{ 0x200, "nVidia Geforce 3", DESKTOP },
	{ 0x201, "nVidia Geforce 3 Titanium 200", DESKTOP },
	{ 0x202, "nVidia Geforce 3 Titanium 500", DESKTOP },
	{ 0x203, "nVidia Quadro DCC", DESKTOP },
	{ 0x250, "nVidia Geforce 4 Ti 4600", DESKTOP },
	{ 0x251, "nVidia Geforce 4 Ti 4400", DESKTOP },
	{ 0x253, "nVidia Geforce 4 Ti 4200", DESKTOP },
	{ 0x258, "nVidia Quadro 4 900 XGL", DESKTOP },
	{ 0x259, "nVidia Quadro 4 750 XGL", DESKTOP },
	{ 0x25a, "nVidia Quadro 4 600 XGL", DESKTOP },
	{ 0x25b, "nVidia Quadro 4 700 XGL", DESKTOP },
	{ 0x280, "nVidia Geforce 4 Ti 4800", DESKTOP },
	{ 0x281, "nVidia Geforce 4 Ti 4200 8X", DESKTOP },
	{ 0x282, "nVidia Geforce 4 Ti 4800SE", DESKTOP },
	{ 0x286, "nVidia Geforce 4 4000 GO", MOBILE },
	{ 0x288, "nVidia Quadro 4 980 XGL", DESKTOP },
	{ 0x289, "nVidia Quadro 4 780 XGL", DESKTOP },
	{ 0x28c, "nVidia Quadro 4 700 GoGL", MOBILE },
	{ 0x300, "nVidia GeforceFX 5800", DESKTOP },
	{ 0x301, "nVidia GeforceFX 5800 Ultra", DESKTOP },
	{ 0x302, "nVidia GeforceFX 5800", DESKTOP },
	{ 0x308, "nVidia QuadroFX 2000", DESKTOP },
	{ 0x309, "nVidia QuadroFX 1000", DESKTOP },
	{ 0x311, "nVidia GeforceFX 5600 Ultra", DESKTOP },
	{ 0x312, "nVidia GeforceFX 5600", DESKTOP },
	{ 0x314, "nVidia GeforceFX 5600XT", DESKTOP },
	{ 0x316, "nVidia NV31", MOBILE },
	{ 0x317, "nVidia NV31", MOBILE },
	{ 0x318, "nVidia NV31GL", DESKTOP },
	{ 0x319, "nVidia NV31GL", DESKTOP },
	{ 0x31a, "nVidia GeforceFX Go 5600", MOBILE },
	{ 0x31b, "nVidia GeforceFX Go 5650", MOBILE },
	{ 0x31c, "nVidia QuadroFX Go700", MOBILE },
	{ 0x31d, "NV31", MOBILE },
	{ 0x31e, "NV31GL", MOBILE },
	{ 0x31f, "NV31GL", MOBILE },
	{ 0x321, "nVidia GeforceFX 5200 Ultra", DESKTOP },
	{ 0x322, "nVidia GeforceFX 5200", DESKTOP },
	{ 0x323, "nVidia GeforceFX 5200LE", DESKTOP },
	{ 0x324, "nVidia GeforceFX Go 5200", MOBILE },
	{ 0x325, "nVidia GeforceFX Go 5250", MOBILE },
	{ 0x326, "nVidia GeforceFX 5500", DESKTOP },
	{ 0x328, "nVidia GeForceFX Go5200 32M/64M", MOBILE },
	{ 0x329, "nVidia GeForce FX 5200 (Mac)", MOBILE },
	{ 0x32a, "nVidia Quadro NVS 280 PCI", DESKTOP },
	{ 0x32b, "nVidia QuadroFX 500", DESKTOP },
	{ 0x32c, "nVidia GeforceFX Go5300", MOBILE },
	{ 0x32d, "nVidia GeforceFX Go5100", MOBILE },
	{ 0x32f, "nVidia NV34GL", DESKTOP },
	{ 0x330, "nVidia GeforceFX 5900 Ultra", DESKTOP },
	{ 0x331, "nVidia GeforceFX 5900", DESKTOP },
	{ 0x332, "nVidia GeforceFX 5900XT", DESKTOP },
	{ 0x333, "nVidia GeforceFX 5950 Ultra", DESKTOP },
	{ 0x334, "nVidia GeforceFX 5900ZT", DESKTOP },
	{ 0x338, "nVidia QuadroFX 3000", DESKTOP },
	{ 0x33f, "nVidia GeforceFX 700", DESKTOP },
	{ 0x341, "nVidia GeforceFX 5700 Ultra", DESKTOP },
	{ 0x342, "nVidia GeforceFX 5700", DESKTOP },
	{ 0x343, "nVidia GeforceFX 5700LE", DESKTOP },
	{ 0x344, "nVidia GeforceFX 5700VE", DESKTOP },
	{ 0x345, "NV36", DESKTOP },
	{ 0x347, "nVidia GeforceFX Go5700", MOBILE },
	{ 0x348, "nVidia GeforceFX Go5700", MOBILE },
	{ 0x349, "NV36", MOBILE },
	{ 0x34b, "NV36", MOBILE },
	{ 0x34c, "nVidia Quadro FX Go1000", MOBILE },
	{ 0x34e, "nVidia QuadroFX 1100", DESKTOP },
	{ 0x34f, "NV36GL", DESKTOP },
	{ 0x2a0, "nVidia Xbox GPU", NFORCE },
	{ 0x40, "nVidia Geforce 6800 Ultra", DESKTOP },
	{ 0x41, "nVidia Geforce 6800", DESKTOP },
	{ 0x42, "nVidia Geforce 6800LE", DESKTOP },
	{ 0x43, "nVidia Geforce 6800XE", DESKTOP },
	{ 0x44, "nVidia Geforce 6800XT", DESKTOP },
	{ 0x45, "nVidia Geforce 6800GT", DESKTOP },
	{ 0x46, "nVidia Geforce 6800GT", DESKTOP },
	{ 0x47, "nVidia Geforce 6800GS", DESKTOP },
	{ 0x48, "nVidia Geforce 6800XT", DESKTOP },
	{ 0x49, "NV40GL", DESKTOP },
	{ 0x4d, "nVidia QuadroFX 4400", DESKTOP },
	{ 0x4e, "nVidia QuadroFX 4000", DESKTOP },
	{ 0xc0, "nVidia Geforce 6800GS", DESKTOP },
	{ 0xc1, "nVidia Geforce 6800", DESKTOP },
	{ 0xc2, "nVidia Geforce 6800LE", DESKTOP },
	{ 0xc3, "nVidia Geforce 6800XT", DESKTOP },
	{ 0xc8, "nVidia Geforce Go 6800", MOBILE },
	{ 0xc9, "nVidia Geforce Go 6800Ultra", MOBILE },
	{ 0xcc, "nVidia QuadroFX Go 1400", MOBILE },
	{ 0xcd, "nVidia QuadroFX 3350/4000SDI", DESKTOP },
	{ 0xce, "nVidia QuadroFX 1400", DESKTOP },
	{ 0xf0, "nVidia Geforce 6800/Ultra", DESKTOP },
	{ 0xf1, "nVidia Geforce 6600/GT", DESKTOP },
	{ 0xf2, "nVidia Geforce 6600", DESKTOP },
	{ 0xf3, "nVidia Geforce 6200", DESKTOP },
	{ 0xf4, "nVidia Geforce 6600LE", DESKTOP },
	{ 0xf5, "nVidia Geforce 7800GS", DESKTOP },
	{ 0xf6, "nVidia Geforce 6800GS/XT", DESKTOP },
	{ 0xf8, "nVidia QuadroFX 3400", DESKTOP },
	{ 0xf9, "nVidia Geforce 6800 Ultra", DESKTOP },
	{ 0xfa, "nVidia GeforcePCX 5750", DESKTOP },
	{ 0xfb, "nVidia GeforcePCX 5900", DESKTOP },
	{ 0xfc, "nVidia GeforcePCX 5300 / Quadro FX330", DESKTOP },
	{ 0xfd, "nVidia Quadro NVS280/FX330", DESKTOP },
	{ 0xfe, "nVidia QuadroFX 1300", DESKTOP },
	{ 0xff, "nVidia GeforcePCX 4300", DESKTOP },
	{ 0x140, "nVidia Geforce 6600GT", DESKTOP },
	{ 0x141, "nVidia Geforce 6600", DESKTOP },
	{ 0x142, "nVidia Geforce 6600LE", DESKTOP },
	{ 0x143, "NV43", DESKTOP },
	{ 0x144, "nVidia Geforce Go 6600", MOBILE },
	{ 0x145, "nVidia Geforce 6610XL", DESKTOP },
	{ 0x146, "nVidia GeForce Go 6600TE/6200TE", MOBILE },
	{ 0x147, "nVidia Geforce 6700XL", DESKTOP },
	{ 0x148, "nVidia Geforce Go 6600", MOBILE },
	{ 0x149, "nVidia Geforce Go 6600GT", MOBILE },
	{ 0x14a, "NV43", DESKTOP },
	{ 0x14b, "NV43", DESKTOP },
	{ 0x14c, "nVidia QuadroFX 550", DESKTOP },
	{ 0x14d, "nVidia QuadroFX 550", DESKTOP },
	{ 0x14e, "nVidia QuadroFX 540", DESKTOP },
	{ 0x14f, "nVidia Geforce 6200", DESKTOP },
	{ 0x160, "nVidia NV44", DESKTOP },
	{ 0x161, "nVidia Geforce 6200 TurboCache", DESKTOP },
	{ 0x162, "nVidia Geforce 6200SE TurboCache", DESKTOP },
	{ 0x163, "NV44", DESKTOP },
	{ 0x164, "nVidia Geforce Go 6200", MOBILE },
	{ 0x165, "nVidia Quadro NVS 285", DESKTOP },
	{ 0x166, "nVidia Geforce Go 6400", MOBILE },
	{ 0x167, "nVidia Geforce Go 6200", MOBILE },
	{ 0x168, "nVidia Geforce Go 6400", MOBILE },
	{ 0x169, "nVidia Geforce 6250", DESKTOP },
	{ 0x16a, "nVidia Geforce 7100GS", DESKTOP },
	{ 0x16b, "NV44GLM", DESKTOP },
	{ 0x16c, "NV44GLM", DESKTOP },
	{ 0x16d, "NV44GLM", DESKTOP },
	{ 0x16e, "NV44GLM", DESKTOP },
	{ 0x210, "NV48", DESKTOP },
	{ 0x211, "nVidia Geforce 6800", DESKTOP },
	{ 0x212, "nVidia Geforce 6800LE", DESKTOP },
	{ 0x215, "nVidia Geforce 6800GT", DESKTOP },
	{ 0x218, "nVidia Geforce 6800XT", DESKTOP },
	{ 0x220, "Unknown NV44", DESKTOP },
	{ 0x221, "nVidia Geforce 6200", DESKTOP },
	{ 0x222, "nVidia Geforce 6200 A-LE", DESKTOP },
	{ 0x228, "NV44M", MOBILE },
	{ 0x240, "nVidia Geforce 6150", NFORCE },
	{ 0x241, "nVidia Geforce 6150LE", NFORCE },
	{ 0x242, "nVidia Geforce 6100", NFORCE },
	{ 0x244, "nVidia Geforce Go 6150", NFORCE },
	{ 0x245, "nVidia Quadro NVS 210S / Geforce 6150LE", NFORCE },
	{ 0x247, "nVidia Geforce Go 6100", NFORCE },
	{ 0x2dd, "nVidia Unknown NV4x", DESKTOP },
	{ 0x2de, "nVidia Unknown NV4x", DESKTOP },
	{ 0x90, "nVidia G70", DESKTOP },
	{ 0x91, "nVidia Geforce 7800GTX", DESKTOP },
	{ 0x92, "nVidia Geforce 7800GT", DESKTOP },
	{ 0x93, "nVidia Geforce 6800GS", DESKTOP },
	{ 0x94, "nVidia G70", DESKTOP },
	{ 0x98, "nVidia G70", DESKTOP },
	{ 0x99, "nVidia Geforce Go 7800GTX", MOBILE },
	{ 0x9c, "nVidia G70", DESKTOP },
	{ 0x9d, "nVidia QuadroFX 4500", DESKTOP },
	{ 0x9e, "nVidia G70GL", DESKTOP },
	{ 0x290, "nVidia Geforce 7900GTX", DESKTOP },
	{ 0x291, "nVidia Geforce 7900GT", DESKTOP },
	{ 0x292, "nVidia Geforce 7900GS", DESKTOP },
	{ 0x293, "nVidia Geforce 7950GX2", DESKTOP },
	{ 0x294, "nVidia Geforce 7950GX2", DESKTOP },
	{ 0x295, "nVidia Geforce 7950GT", DESKTOP },
	{ 0x297, "nVidia Geforce Go 7950GTX", MOBILE },
	{ 0x298, "nVidia Geforce Go 7900GS", MOBILE },
	{ 0x299, "nVidia Geforce Go 7900GTX", MOBILE },
	{ 0x29a, "nVidia QuadroFX 2500M", MOBILE },
	{ 0x29b, "nVidia QuadroFX 1500M", MOBILE },
	{ 0x29c, "nVidia QuadroFX 5500", DESKTOP },
	{ 0x29d, "nVidia QuadroFX 3500", DESKTOP },
	{ 0x29e, "nVidia QuadroFX 1500", DESKTOP },
	{ 0x29f, "nVidia QuadroFX 4500 X2", DESKTOP },
	{ 0x390, "nVidia Geforce 7650GS", DESKTOP },
	{ 0x391, "nVidia Geforce 7600GT", DESKTOP },
	{ 0x392, "nVidia Geforce 7600GS", DESKTOP },
	{ 0x393, "nVidia Geforce 7300GT", DESKTOP },
	{ 0x394, "nVidia Geforce 7600LE", DESKTOP },
	{ 0x395, "nVidia Geforce 7300GT", DESKTOP },
	{ 0x397, "nVidia Geforce Go 7700", MOBILE },
	{ 0x398, "nVidia Geforce Go 7600", MOBILE },
	{ 0x399, "nVidia Geforce Go 7600GT", MOBILE },
	{ 0x39a, "nVidia Quadro NVS 300M", MOBILE },
	{ 0x39b, "nVidia Geforce Go 7900SE", MOBILE },
	{ 0x39c, "nVidia QuadroFX 550M", MOBILE },
	{ 0x39e, "nVidia QuadroFX 560", DESKTOP },
	{ 0x2e0, "nVidia Geforce 7600GT", DESKTOP },
	{ 0x2e1, "nVidia Geforce 7600GS", DESKTOP },
	{ 0x2e2, "nVidia Geforce 7300GT", DESKTOP },
	{ 0x2e4, "nVidia Geforce 7950GT", DESKTOP },
	{ 0x1d1, "nVidia Geforce 7300LE", DESKTOP },
	{ 0x1d3, "nVidia Geforce 7300SE", DESKTOP },
	{ 0x1d7, "nVidia Geforce Go 7300", MOBILE },
	{ 0x1d8, "nVidia Geforce Go 7400", MOBILE },
	{ 0x1d9, "nVidia Geforce Go 7400GS", MOBILE },
	{ 0x1da, "nVidia Quadro NVS 110M", MOBILE },
	{ 0x1db, "nVidia Quadro NVS 120M", MOBILE },
	{ 0x1dc, "nVidia QuadroFX 350M", MOBILE },
	{ 0x1dd, "nVidia Geforce 7500LE", MOBILE },
	{ 0x1de, "nVidia QuadroFX 350", DESKTOP },
	{ 0x1df, "nVidia Geforce 7300GS", DESKTOP },
	{ 0x190, "nVidia Geforce 8800", DESKTOP },
	{ 0x191, "nVidia Geforce 8800GTX", DESKTOP },
	{ 0x192, "nVidia Geforce 8800", DESKTOP },
	{ 0x193, "nVidia Geforce 8800GTS", DESKTOP },
	{ 0x194, "nVidia Geforce 8800Ultra", DESKTOP },
	{ 0x197, "nVidia Geforce 8800", DESKTOP },
	{ 0x19a, "nVidia G80-875", DESKTOP },
	{ 0x19d, "nVidia QuadroFX 5600", DESKTOP },
	{ 0x19e, "nVidia QuadroFX 4600", DESKTOP },
	{ 0x400, "nVidia Geforce 8600GTS", DESKTOP },
	{ 0x401, "nVidia Geforce 8600GT", DESKTOP },
	{ 0x402, "nVidia Geforce 8600GT", DESKTOP },
	{ 0x403, "nVidia Geforce 8600GS", DESKTOP },
	{ 0x404, "nVidia Geforce 8400GS", DESKTOP },
	{ 0x405, "nVidia Geforce 9500M GS", MOBILE },
	{ 0x406, "nVidia Geforce NB9P-GE", MOBILE },
	{ 0x407, "nVidia Geforce 8600M GT", MOBILE },
	{ 0x408, "nVidia Geforce 8600M GTS", MOBILE },
	{ 0x409, "nVidia Geforce 8700M GT", MOBILE },
	{ 0x40a, "nVidia Quadro NVS 370M", MOBILE },
	{ 0x40b, "nVidia Quadro NVS 320M", MOBILE },
	{ 0x40c, "nVidia QuadroFX 570M", MOBILE },
	{ 0x40d, "nVidia QuadroFX 1600M", MOBILE },
	{ 0x40e, "nVidia QuadroFX 570", DESKTOP },
	{ 0x40f, "nVidia QuadroFX 1700", DESKTOP },
	{ 0x420, "nVidia Geforce 8400SE", DESKTOP },
	{ 0x421, "nVidia Geforce 8500GT", DESKTOP },
	{ 0x422, "nVidia Geforce 8400GS", DESKTOP },
	{ 0x423, "nVidia Geforce 8300GS", DESKTOP },
	{ 0x424, "nVidia Geforce 8400GS", DESKTOP },
	{ 0x425, "nVidia Geforce 8600M GS", MOBILE },
	{ 0x426, "nVidia Geforce 8400M GT", MOBILE },
	{ 0x427, "nVidia Geforce 8400M GS", MOBILE },
	{ 0x428, "nVidia Geforce 8400M G", MOBILE },
	{ 0x429, "nVidia Quadro NVS 140M", MOBILE },
	{ 0x42a, "nVidia Quadro NVS 130M", MOBILE },
	{ 0x42b, "nVidia Quadro NVS 135M", MOBILE },
	{ 0x42d, "nVidia Quadro FX 360M", MOBILE },
	{ 0x42e, "nVidia Geforce 9300M G", MOBILE },
	{ 0x42f, "nVidia Quadro NVS 290", DESKTOP },
	{ 0x3d0, "nVidia Geforce 6100 nForce 430", NFORCE},
	{ 0x3d1, "nVidia Geforce 6100 nForce 405", NFORCE},
	{ 0x3d2, "nVidia Geforce 6100 nForce 400", NFORCE},
	{ 0x3d5, "nVidia Geforce 6100 nForce 420", NFORCE},
	{ 0x53a, "nVidia Geforce 7050PV nForce 630a", NFORCE},
	{ 0x53b, "nVidia Geforce 7050PV nForce 630a", NFORCE},
	{ 0x53e, "nVidia Geforce 7025 nForce 630a", NFORCE},
	{ 0x5e0, "nvidia GeForce GT200-400", DESKTOP },
	{ 0x5e1, "nvidia GeForce GTX 280", DESKTOP },
	{ 0x5e2, "nvidia GeForce GTX 260", DESKTOP },
	{ 0x5e3, "nvidia GeForce GTX 285", DESKTOP },
	{ 0x5e6, "nvidia GeForce GTX 275", DESKTOP },
	{ 0x5e7, "nvidia Tesla C1060", DESKTOP },
	{ 0x5ed, "nvidia Quadroplex 2200 D2", DESKTOP },
	{ 0x5f8, "nvidia Quadroplex 2200 S4", DESKTOP },
	{ 0x5f9, "nvidia Quadro CX", DESKTOP },
	{ 0x5fd, "nvidia QuadroFX 5800", DESKTOP },
	{ 0x5fe, "nvidia QuadroFX 5800", DESKTOP },
	{ 0x600, "nVidia Geforce 8800GTS 512", DESKTOP },
	{ 0x602, "nVidia Geforce 8800GT", DESKTOP },
	{ 0x604, "nVidia Geforce 9800GX2", DESKTOP },
	{ 0x606, "nVidia Geforce 8800GS", DESKTOP },
	{ 0x60d, "nVidia Geforce 8800GS", DESKTOP },
	{ 0x609, "nVidia Geforce 8800M GTS", MOBILE },
	{ 0x60c, "nVidia Geforce 8800M GTX", MOBILE },
	{ 0x610, "nVidia Geforce 9600GSO", DESKTOP },
	{ 0x611, "nVidia Geforce 8800GT", DESKTOP },
	{ 0x612, "nVidia Geforce 9800GTX", DESKTOP },
	{ 0x614, "nVidia Geforce 9800GT", DESKTOP },
	{ 0x615, "nVidia Geforce GTS250", DESKTOP },
	{ 0x61a, "nVidia QuadroFX 3700", DESKTOP },
	{ 0x61c, "nVidia QuadroFX 3600M", MOBILE },
	{ 0x622, "nVidia Geforce 9600GT", DESKTOP },
	{ 0x623, "nVidia Geforce 9600GS", DESKTOP },
	{ 0x640, "nVidia Geforce 9500GT", DESKTOP },
	{ 0x643, "nVidia Geforce 9500GT", DESKTOP },
	{ 0x647, "nVidia Geforce 9600M GT", MOBILE },
	{ 0x648, "nVidia Geforce 9600M GS", MOBILE },
	{ 0x649, "nVidia Geforce 9600M GT", MOBILE },
	{ 0x64b, "nVidia Geforce 9500M G", MOBILE },
	{ 0x6e0, "nVidia Geforce 9300GE", DESKTOP },
	{ 0x6e1, "nVidia Geforce 9300GS", DESKTOP },
	{ 0x6e2, "nVidia Geforce 8400", DESKTOP },
	{ 0x6e3, "nVidia Geforce 8400SE", DESKTOP },
	{ 0x6e4, "nVidia Geforce 8400GS", DESKTOP },
	{ 0x6e5, "nVidia Geforce 9300M GS", MOBILE },
	{ 0x6e6, "nVidia Geforce G100", DESKTOP },
	{ 0x6e7, "nVidia G98", DESKTOP },
	{ 0x6e8, "nVidia Geforce 9200M GS", MOBILE },
	{ 0x6e9, "nVidia Geforce 9300M GS", MOBILE },
	{ 0x6ea, "nVidia Quadro NVS 150M", MOBILE },
	{ 0x6eb, "nVidia Quadro NVS 160M", MOBILE },
	{ 0x6ec, "nVidia Geforce G105M", MOBILE },
	{ 0x6ed, "nVidia G98", DESKTOP },
	{ 0x6ee, "nVidia G98", DESKTOP },
	{ 0x6ef, "nVidia G98", DESKTOP },
	{ 0x6f0, "nVidia G98", DESKTOP },
	{ 0x6f1, "nVidia G98", DESKTOP },
	{ 0x6f2, "nVidia G98", DESKTOP },
	{ 0x6f3, "nVidia G98", DESKTOP },
	{ 0x6f4, "nVidia G98", DESKTOP },
	{ 0x6f5, "nVidia G98", DESKTOP },
	{ 0x6f6, "nVidia G98", DESKTOP },
	{ 0x6f7, "nVidia G98", DESKTOP },
	{ 0x6f8, "nVidia Quadro NVS 420", DESKTOP },
	{ 0x6f9, "nVidia QuadroFX 370 LP", DESKTOP },
	{ 0x6fa, "nVidia Quadro NVS 450", DESKTOP },
	{ 0x6fb, "nVidia QuadroFX 370M", MOBILE },
	{ 0x6fc, "nVidia G98", DESKTOP },
	{ 0x6fd, "nVidia Quadro NVS 295", DESKTOP },
	{ 0x6fe, "nVidia G98", DESKTOP },
	{ 0x6ff, "nVidia G98-GL", DESKTOP },
	{ 0x860, "nVidia Geforce 9300", DESKTOP },
	{ 0x861, "nVidia Geforce 9400", DESKTOP },
	{ 0x863, "nVidia Geforce 9400M", MOBILE },
	{ 0x864, "nVidia Geforce 9300", DESKTOP },
	{ 0x865, "nVidia Geforce 9300", DESKTOP },
	{ 0, NULL, UNKNOWN }
};

const char *get_card_name(int device_id, gpu_type *gpu)
{
	struct pci_ids *nv_ids = (struct pci_ids*)ids;

	while(nv_ids->id != 0)
	{
		if(nv_ids->id == device_id)
		{
			*gpu = nv_ids->gpu;
			return nv_ids->name;
		}

		nv_ids++;
	}

	/* if !found */
	*gpu = UNKNOWN;
	return "Unknown Nvidia card";
}

/* Internal gpu architecture function which sets
/  a device to a specific architecture. This architecture
/  doesn't have to be the real architecture. It is mainly
/  used to choose codepaths inside nvclock.
*/
int get_gpu_arch(int device_id)
{
	int arch;
	switch(device_id & 0xff0)
	{
		case 0x20:
			arch = NV5;
			break;
		case 0x100:
		case 0x110:
		case 0x150:
		case 0x1a0:
			arch = NV10;
			break;
		case 0x170:
		case 0x180:
		case 0x1f0:
			arch = NV17;
			break;
		case 0x200:
			arch = NV20;
			break;
		case 0x250:
		case 0x280:
		case 0x320:	/* We don't treat the FX5200/FX5500 as FX cards */
			arch = NV25;
			break;
		case 0x300:
			arch = NV30;
			break;
		case 0x330:
			arch = NV35; /* Similar to NV30 but fanspeed stuff works differently */
			break;
		/* Give a seperate arch to FX5600/FX5700 cards as they need different code than other FX cards */
		case 0x310:
		case 0x340:
			arch = NV31;
			break;
		case 0x40:
		case 0x120:
		case 0x130:
		case 0x210:
		case 0x230:
			arch = NV40;
			break;
		case 0xc0:
			arch = NV41;
			break;
		case 0x140:
			arch = NV43; /* Similar to NV40 but with different fanspeed code */
			break;
		case 0x160:
		case 0x220:
			arch = NV44;
			break;
		case 0x1d0:
			arch = NV46;
			break;
		case 0x90:
			arch = NV47;
			break;
		case 0x290:
			arch = NV49; /* 7900 */
			break;
		case 0x390:
			arch = NV4B; /* 7600 */
			break;
		case 0x190:
			arch = NV50; /* 8800 'NV50 / G80' */
			break;
		case 0x400: /* 8600 'G84' */
			arch = G84;
			break;
		case 0x420: /* 8500 'G86' */
			arch = G86;
			break;
		case 0x5e0: /* GT2x0 */
		case 0x5f0: /* GT2x0 */
		case 0xa60: /* GT2x0 */
		case 0xa20:
		case 0xa30:
		case 0xa70:
		case 0xca0:
			arch = GT200;
			break;
		case 0x6e0: /* G98 */
		case 0x6f0: /* G98 */
		case 0x860: /* C79 */
			arch = G86;
			break;
		case 0x600: /* G92 */
		case 0x610: /* G92 */
			arch = NV50;
			break;
		case 0x620: /* 9600GT 'G94' */
			arch = G94;
			break;
		case 0x640: /* 9500GT */
			arch = G96;
			break;
		case 0x240:
		case 0x3d0: /* not sure if this is a C51 too */
		case 0x530: /* not sure if the 70xx is C51 too */
			arch = C51;
			break;
		case 0x2e0:
		case 0xf0:
			/* The code above doesn't work for pci-express cards as multiple architectures share one id-range */
			switch(device_id)
			{
				case 0xf0: /* 6800 */
				case 0xf9: /* 6800Ultra */
					arch = NV40;
					break;
				case 0xf6: /* 6800GS/XT */
					arch = NV41;
					break;
				case 0xf1: /* 6600/6600GT */
				case 0xf2: /* 6600GT */
				case 0xf3: /* 6200 */
				case 0xf4: /* 6600LE */
					arch = NV43;
					break;
				case 0xf5: /* 7800GS */
					arch = NV47;
					break;
				case 0xfa: /* PCX5700 */
					arch = NV31;
					break;
				case 0xf8: /* QuadroFX 3400 */
				case 0xfb: /* PCX5900 */
					arch = NV35;
					break;
				case 0xfc: /* PCX5300 */
				case 0xfd: /* Quadro NVS280/FX330, FX5200 based? */
				case 0xff: /* PCX4300 */
					arch = NV25;
					break;
				case 0xfe: /* Quadro 1300, has the same id as a FX3000 */
					arch = NV35;
					break;
				case 0x2e0: /* Geforce 7600GT AGP (at least Leadtek uses this id) */
				case 0x2e1: /* Geforce 7600GS AGP (at least BFG uses this id) */
				case 0x2e2: /* Geforce 7300GT AGP (at least a Galaxy 7300GT uses this id) */
					arch = NV4B;
					break;
				case 0x2e4: /* Geforce 7950 GT AGP */
					arch = NV49;
					break;
			}
			break;
		default:
			arch = UNKNOWN;
	}
	return arch;
}

/* Receive the real gpu architecture */
static short get_gpu_architecture()
{
	return (nv_read_pmc(NV_PMC_BOOT_0) >> 20) & 0xff;
}

/* Receive the gpu revision */
static short get_gpu_revision()
{
	return nv_read_pmc(NV_PMC_BOOT_0) & NV_PMC_BOOT_0_REVISION_MASK;
}

/* Retrieve the 'real' PCI id from the card */
static short get_gpu_pci_id()
{
	return nv_read_pbus16(PCI_DEVICE_ID);
}

/* Retrieve the pci subvendor id */
static short get_pci_subvendor_id()
{
	return nv_read_pbus16(PCI_SUBSYSTEM_VENDOR_ID);
}

static int set_gpu_pci_id(short id)
{
	if(nv_card->arch & (NV10 | NV20))
	{
		/* The first two bits of the pci id can be changed. They are stored in bit 13-12 of PEXTDEV_BOOT0 */
		int pextdev_boot0 = nv_card->PEXTDEV[0x0/4] & ~(0x3 << 12);
		/* Only the first 2 bits can be changed on these GPUs */
		if(id > 3)
			return 0;

		nv_card->PEXTDEV[0x0/4] = pextdev_boot0 | ((id & 0x3) << 12);
		nv_card->device_id = get_gpu_pci_id();
		nv_card->card_name = (char*)get_card_name(nv_card->device_id, &nv_card->gpu);
		return 1;
	}
	/* Don't allow modding on cards using bridges (0xf*)! */
	else if((nv_card->arch & (NV17 | NV25 | NV3X | NV4X)) && ((nv_card->device_id & 0xfff0) != 0xf0))
	{
		/* The first four bits of the pci id can be changed. The first two bits are stored in bit 13-12 of PEXTDEV_BOOT0, bit 3 and 4 are stored in bit 21-20  */
		int pextdev_boot0 = nv_card->PEXTDEV[0x0/4] & ~((0x3 << 20) | (0x3 << 12));
		/* The first 4 bits can be changed */
		if(id > 16)
			return 0;
		
		/* On various NV4x cards the quadro capability bit in PBUS_DEBUG1 is locked. It can be unlocked by setting the first bit in 0xc020/0xc028 */
		nv_card->PMC[0xc020/4] = 1;
		nv_card->PMC[0xc028/4] = 1;

		nv_card->PEXTDEV[0x0/4] = pextdev_boot0 | (((id>>2) & 0x3) << 20) | ((id & 0x3) << 12);
		nv_card->device_id = get_gpu_pci_id();
		nv_card->card_name = (char*)get_card_name(nv_card->device_id, &nv_card->gpu);
		return 1;
	}
	return 0;
}

/* Function to read a single byte from pci configuration space */
static void read_byte(int offset, unsigned char *data)
{
	/* The original plan was to read the PCI configuration directly from registers 0x1800 and upwards
	/  from the card itself. Although this is a fully correct way, it doesn't work for some cards using
	/  a PCI-Express -> AGP bridge. If I would read the registers from the card they would include PCI-Express
	/  as one of the capabilities. Reading using the "normal" way results in AGP as one of the capabilities.
	/  To correctly show that a card uses AGP we need to read the modded config space.
	*/
	*data = pciReadLong(nv_card->devbusfn,offset) & 0xff;
}

/* Check the videocard for a certain PCI capability like AGP/PCI-Express/PowerManagement.
/  If a certain capability is supported return the position of the cap pointer. 
*/
static int pci_find_capability(unsigned char cap)
{
	unsigned char pos, id;

	read_byte(PCI_CAPABILITY_LIST, &pos);

	while(pos >= 0x40)
	{
		pos &= ~3;
		read_byte(pos + PCI_CAP_LIST_ID, &id);
		if(id == 0xff)
			break;
		if(id == cap)
			return pos; /* Return the position of the cap pointer */

		read_byte(pos + PCI_CAP_LIST_NEXT, &pos);
	}
	return 0;
}

/* Check the videocard for a certain PCI capability like AGP/PCI-Express/PowerManagement.
/  If a certain capability is supported return the position of the cap pointer. 
*/
static int nv_pci_find_capability(unsigned char cap)
{
	unsigned char pos, id;

	pos = nv_read_pbus8(PCI_CAPABILITY_LIST);

	while(pos >= 0x40)
	{
		pos &= ~3;
		id = nv_read_pbus8(pos + PCI_CAP_LIST_ID);
		if(id == 0xff)
			break;
		if(id == cap)
			return pos; /* Return the position of the cap pointer */

		pos = nv_read_pbus8(pos + PCI_CAP_LIST_NEXT);
	}
	return 0;
}

char* get_bus_type()
{
	/* The pci header contains lots of information about a device like
	/  what type of device it is, who the vendor is and so on. It also
	/  contains a list of capabilities. Things like AGP, power management,
	/  PCI-X and PCI-Express are considered capabilities. We could check
	/  these capabilities to find out if the card is AGP or PCI-Express.
	/
	/  Reading the bus type from the pci header would be a nice way but
	/  unfortunately there are some issues. One way to do the reading
	/  is to read the information directly from the card (from PMC).
	/  This doesn't work as some PCI-Express boards (6600GT) actually use
	/  PCI-Express GPUs connected to some bridge chip on AGP boards (same device id!).
	/  If you read directly from the card it will advertise PCI-Express instead of AGP.
	/  There is also another way to read the pci header for instance on Linux
	/  using from files in /proc/bus/pci but non-root users can only read
	/  a small part of the file. Most of the time the info we need isn't in
	/  the readable part. Further there are also some early PCI-Express boards (GeforcePCX)
	/  that contain bridge chips to turn AGP GPUs into PCI-Express ones.
	/
	/  Currently we will return PCI-Express on GeforcePCX board under the valid
	/  assumption that there are no AGP boards with the same device id. Further
	/  it seems that 'low' device ids are for PCI-Express->AGP while the higher ones
	/  are for AGP->PCI-Express, so for the lower ones (6200/6600/6800) we will return AGP
	/  and for the higher ones PCI-Express. A nicer way would be to read all this stuff from
	/  the pci header but as explained that can't be done at the moment.
	*/
	switch(nv_card->device_id)
	{
		case 0xf0: /* 6800 */
		case 0xf1: /* 6600GT */
		case 0xf2: /* 6600 */
		case 0xf3: /* 6200 */
		case 0xf5: /* 6800GS/XT */
		case 0xf6: /* 7800GS */
		case 0x2e0: /* 7600GT */
		case 0x2e1: /* 7600GS */
		case 0x2e2: /* 7300GT */
			return STRDUP("AGP (BR02)", sizeof("AGP (BR02)")); /* We return something different from AGP for now as we don't want to show the AGP tab */
		case 0xf8: /* Quadro FX3400 */
		case 0xf9: /* Geforce 6800 series */
		case 0xfa: /* PCX5500 */
		case 0xfb: /* PCX5900 */
		case 0xfc: /* Quadro FX330*/
		case 0xfd: /* PCX5500 */
		case 0xfe: /* Quadro 1300 */
		case 0xff: /* PCX4300 */
			return STRDUP("PCI-Express (BR02)", sizeof("PCI-Express (BR02)"));
	}

	if(nv_pci_find_capability(PCI_CAP_ID_EXP))
		return STRDUP("PCI-Express", sizeof("PCI-Express"));
	else if(nv_pci_find_capability(PCI_CAP_ID_AGP))
		return STRDUP("AGP", sizeof("AGP"));
	else
		return STRDUP("PCI", sizeof("PCI"));
}

/* Needs better bus checks .. return a string ?*/
static short get_agp_bus_rate()
{
	int agp_capptr, agp_rate, agp_status;

	agp_capptr = nv_pci_find_capability(PCI_CAP_ID_AGP);
	agp_status = nv_read_pbus(agp_capptr + PCI_AGP_STATUS);
	agp_rate = nv_read_pbus(agp_capptr + PCI_AGP_COMMAND) & PCI_AGP_STATUS_RATE_MASK;

	/* If true, the user has AGP8x support */
	if(agp_status & PCI_AGP_STATUS_RATE_8X_SUPPORT)
	{
		agp_rate <<= PCI_AGP_STATUS_RATE_8X_SHIFT;
	}
	return agp_rate;
}

char* get_agp_fw_status()
{
	int agp_capptr = nv_pci_find_capability(PCI_CAP_ID_AGP);
	unsigned int agp_status = nv_read_pbus(agp_capptr + PCI_AGP_STATUS);
	unsigned int agp_command = nv_read_pbus(agp_capptr + PCI_AGP_COMMAND);

	/* Check if Fast Writes is supported by the hostbridge */
	if(agp_status & PCI_AGP_STATUS_FW)
		return (agp_command & PCI_AGP_COMMAND_FW) ? STRDUP("Enabled", sizeof("Enabled")) : STRDUP("Disabled", sizeof("Disabled"));
	else
		return STRDUP("Unsupported", sizeof("Unsupported"));
}

char* get_agp_sba_status()
{
	int agp_capptr = nv_pci_find_capability(PCI_CAP_ID_AGP);
	unsigned int agp_status = nv_read_pbus(agp_capptr + PCI_AGP_STATUS);
	unsigned int agp_command = nv_read_pbus(agp_capptr + PCI_AGP_COMMAND);

	/* Check if Sideband Addressing is supported by the hostbridge */
	if(agp_status & PCI_AGP_STATUS_SBA)
		return (agp_command & PCI_AGP_COMMAND_SBA) ? STRDUP("Enabled", sizeof("Enabled")) : STRDUP("Disabled", sizeof("Disabled"));
	else
		return STRDUP("Unsupported", sizeof("Unsupported"));
}

char* get_agp_status()
{
	int agp_capptr = nv_pci_find_capability(PCI_CAP_ID_AGP);
	unsigned int agp_command = nv_read_pbus(agp_capptr + PCI_AGP_COMMAND);
	return (agp_command & PCI_AGP_COMMAND_AGP) ? STRDUP("Enabled", sizeof("Enabled")) : STRDUP("Disabled", sizeof("Disabled"));
}

static char* get_agp_supported_rates()
{
	int agp_capptr, agp_rates, agp_status, i;
	static char *rate;

	agp_capptr = nv_pci_find_capability(PCI_CAP_ID_AGP);
	agp_status = nv_read_pbus(agp_capptr + PCI_AGP_STATUS);
	agp_rates = agp_status & PCI_AGP_STATUS_RATE_MASK;

	/* If true, the user has AGP8x support */
	if(agp_status & PCI_AGP_STATUS_RATE_8X_SUPPORT)
	{
		agp_rates <<= PCI_AGP_STATUS_RATE_8X_SHIFT;
	}

	rate = new char;

	for(i=1; i <= 8; i*=2)
	{
		if(agp_rates & i)
		{
			char *temp = new char[4];
			snprintf(temp, 4, "%dX ", i);
			char* newrate=new char[strlen(rate)+4];
			strncpy(newrate, rate, strlen(rate)+4);
			delete[] rate;
			rate=newrate;
			rate = strncat(rate, temp, strlen(rate));
			delete[] temp;
		}
	}

	return rate;
}

static short get_pcie_bus_rate()
{
	int pcie_rate, pcie_status_reg;

	pcie_status_reg = nv_pci_find_capability(PCI_CAP_ID_EXP);
	if(pcie_status_reg != 0 )
	{
		pcie_rate = (nv_read_pbus16(pcie_status_reg + PCIE_LINKSTATUS) & PCIE_LINK_SPEED_MASK) >> PCIE_LINK_SPEED_SHIFT;
		return pcie_rate;
	}
	return 0;
}

static short get_pcie_max_bus_rate()
{
	int pcie_rate, pcie_status_reg;

	pcie_status_reg = nv_pci_find_capability(PCI_CAP_ID_EXP);
	if(pcie_status_reg != 0 )
	{
		pcie_rate = (nv_read_pbus16(pcie_status_reg + PCIE_LINKCAP) & PCIE_LINK_SPEED_MASK) >> PCIE_LINK_SPEED_SHIFT;

		return pcie_rate;
	}
	return 0;
}

static short get_memory_width()
{
	/* Nforce / Nforce2 */
	if((nv_card->device_id == 0x1a0) || (nv_card->device_id == 0x1f0))
		return 64;
	/* GeforceFX cards (except for FX5200) need a different check */
	/* What to do with NV40 cards ? */
	else if(nv_card->arch & NV3X)
	{
		/* I got this info from the rivatuner forum. On the forum
 		*  is a thread containing register dumps from lots of cards.
 		*  It might not be 100% correct but it is better than a pci id check */
		switch(nv_card->PFB[0x200/4] & 0x7)
		{
			/* 64bit FX5600 */
			case 0x1:
				return 64;
			/* 128bit FX5800 */
			case 0x3:
				return 128;
			/* 128bit FX5600, FX5700 */
			case 0x5:
				return 128;
			/* 256bit FX5900 */
			case 0x7:
				return 256;
		}
	}
	else if(nv_card->arch == NV44)
	{
		return 64; /* For now return 64; (Turbocache cards) */
	}
	else if(nv_card->arch & NV4X)
	{
		/* Memory bandwith detection for nv40 but not sure if it is correct, it is atleast better than nothing */
		switch(nv_card->PFB[0x200/4] & 0x7)
		{
			/* 128bit 6600GT */
			case 0x1:
				return 128;
			/* 256bit 6800 */
			case 0x3:
				return 256;
			default:
				return 128;
		}
	}
	else if(nv_card->arch & NV5X)
	{
		/* On Geforce 8800GTS/GTX and 8600GT/GTS cards the memory bandwith is proportional to the number of ROPs * 16.
		*  In case of the 8500 this isn't the case, there the size is just 128 where there are 4 ROPs.
		*  So for now use the number of ROPs as a meassure for the bus width.
		*/
		char rmask, rmask_default;
		switch(nv_card->get_rop_units(&rmask, &rmask_default))
		{
			case 32: /* Geforce GTX280 */
				return 512;
			case 28: /* Geforce GTX260 */
				return 448;
			case 24: /* 8800GTX */
				return 384;
			case 20: /* 8800GTS */
				return 320;
			case 16: /* 8800GT */
				return 256;
			case 12: /* 8800GS */
				return 192;
			case 8: /* 8600GT/GTS */
			case 4: /* 8500GT; 8400GS boards use the same core and offer 64-bit, how to handle this? */
				return 128;
			case 2: /* 8300GS */
				return 64;
		}
	}
	/* Generic algorithm for cards up to the Geforce4 */
	return (nv_card->PEXTDEV[0x0/4] & 0x17) ? 128 : 64;
}

char* get_memory_type()
{
	/* Nforce / Nforce2 */
	if((nv_card->device_id == 0x1a0) || (nv_card->device_id == 0x1f0))
		return ((pciReadLong(0x1, 0x7c) >> 12) & 0x1) ? STRDUP("DDR", sizeof("DDR")) : STRDUP("SDR", sizeof("SDR"));
	else if(nv_card->arch & (NV2X | NV3X))
	{
		/* Based on statistics found on the rivatuner forum, the first two bytes of
		* register 0x1218 of NV2X/NV3X boards, contains "0x0001 or 0x0101" in case of DDR memory and "0x0301" for DDR2.
		*/
		return (((nv_card->PMC[0x1218/4] >> 8) & 0x3) == 0x3) ? STRDUP("DDR2", sizeof("DDR2")) : STRDUP("DDR", sizeof("DDR"));
	}
	else if(nv_card->arch & (NV4X))
	{
		/* On Geforce6/7 cards 0x100474 (PFB 0x474) can be used to distinguish between DDR and DDR3. 
		* Note these values are based on the bios and it was noted that for instance bits in this register differ.
		* In case of DDR3 the first byte contains 0x4 while in case of DDR it contains 0x1.
		*/
		return (nv_card->PFB[0x474/4] & 0x4) ? STRDUP("DDR3", sizeof("DDR3")) : STRDUP("DDR", sizeof("DDR"));
	}
	else if(nv_card->arch & (NV5X))
	{
		/* For now use 0x100218 (PFB 0x218) to distinguish between DDR2 and DDR3. The contents of this
		*  register differs between a 8500GT (DDR2) and 8600GTS/8800GTS (DDR3) according to the bios.
		*  FIXME: use a better register
		*/
		return (nv_card->PFB[0x218/4] & 0x1000000) ? STRDUP("DDR3", sizeof("DDR3")) : STRDUP("DDR2", sizeof("DDR2"));
	}
	else
		/* TNT, Geforce1/2/4MX */
		return (nv_card->PFB[0x200/4] & 0x01) ? STRDUP("DDR", sizeof("DDR")) : STRDUP("SDR", sizeof("SDR"));
}

static short get_memory_size()
{
	short memory_size;

	/* If the card is something TNT based the calculation of the memory is different. */
	if(nv_card->arch == NV5)
	{
		if(nv_card->PFB[0x0/4] & 0x100)
			memory_size = ((nv_card->PFB[0x0/4] >> 12) & 0xf)*2+2;
		else
		{
			switch(nv_card->PFB[0x0/4] & 0x3)
			{
				case 0:
					memory_size = 32;
					break;
				case 1:
					memory_size = 4;
					break;
				case 2:
					memory_size = 8;
					break;
				case 3:
					memory_size = 16;
					break;
				default:
					memory_size = 16;
					break;
			}
		}
	}
	/* Nforce 1 */
	else if(nv_card->device_id == 0x1a0)
	{
		int32_t temp = pciReadLong(0x1, 0x7c);
		memory_size = ((temp >> 6) & 0x31) + 1;
	}
	/* Nforce2 */
	else if(nv_card->device_id == 0x1f0)
	{
		int32_t temp = pciReadLong(0x1, 0x84);
		memory_size = ((temp >> 4) & 0x127) + 1;
	}
	/* Memory calculation for geforce cards or better.*/
	else
	{
		/* The code below is needed to show more than 256MB of memory
		/  but I'm not sure if 0xfff is safe for pre-geforcefx cards.
		/  There's no clean way right now to use 0xff for those old cards
		/  as currently the FX5200/FX5500 (which support 256MB) use the
		/  pre-geforcefx backend.
		*/
		memory_size = (nv_card->PFB[0x20c/4] >> 20) & 0xfff;
	}

	return memory_size;
}

/* Print various GPU registers for debugging purposes */
static void get_debug_info()
{
	printf("--- %s GPU registers ---\n", nv_card->card_name);
	printf("NV_PMC_BOOT_0 (0x0): %08x\n", nv_card->PMC[0]);
	printf("NV_PBUS_DEBUG_0 (0x1080): %08x\n", nv_card->PMC[0x1080/4]);
	printf("NV_PBUS_DEBUG_1 (0x1084): %08x\n", nv_card->PMC[0x1084/4]);
	printf("NV_PBUS_DEBUG_2 (0x1088): %08x\n", nv_card->PMC[0x1088/4]);
	printf("NV_PBUS_DEBUG_3 (0x108c): %08x\n", nv_card->PMC[0x108c/4]);
	printf("NV_10F0 (0x10f0): %08x\n", nv_card->PMC[0x10f0/4]);
	printf("NV_1540 (0x1540): %08x\n", nv_card->PMC[0x1540/4]);
	printf("NV_15B0 (0x15b0): %08x\n", nv_card->PMC[0x15b0/4]);
	printf("NV_15B4 (0x15b4): %08x\n", nv_card->PMC[0x15b4/4]);
	printf("NV_15B8 (0x15b8): %08x\n", nv_card->PMC[0x15b8/4]);
	printf("NV_15F0 (0x15f0): %08x\n", nv_card->PMC[0x15f0/4]);
	printf("NV_15F4 (0x15f4): %08x\n", nv_card->PMC[0x15f4/4]);
	printf("NV_15F8 (0x15f8): %08x\n", nv_card->PMC[0x15f8/4]);
	printf("NV_PBUS_PCI_0 (0x1800): %08x\n", nv_read_pbus(PCI_VENDOR_ID));
	printf("NV_PBUS_PCI_0 (0x182c): %08x\n", nv_read_pbus(PCI_SUBSYSTEM_VENDOR_ID));

	if(nv_card->arch & (NV4X | NV5X))
	{
		printf("NV_C010 (0xc010): %08x\n", nv_card->PMC[0xc010/4]);
		printf("NV_C014 (0xc014): %08x\n", nv_card->PMC[0xc014/4]);
		printf("NV_C018 (0xc018): %08x\n", nv_card->PMC[0xc018/4]);
		printf("NV_C01C (0xc01c): %08x\n", nv_card->PMC[0xc01c/4]);
		printf("NV_C020 (0xc020): %08x\n", nv_card->PMC[0xc020/4]);
		printf("NV_C024 (0xc024): %08x\n", nv_card->PMC[0xc024/4]);
		printf("NV_C028 (0xc028): %08x\n", nv_card->PMC[0xc028/4]);
		printf("NV_C02C (0xc02c): %08x\n", nv_card->PMC[0xc02c/4]);
		printf("NV_C040 (0xc040): %08x\n", nv_card->PMC[0xc040/4]);
		printf("NV_4000 (0x4000): %08x\n", nv_card->PMC[0x4000/4]);
		printf("NV_4004 (0x4004): %08x\n", nv_card->PMC[0x4004/4]);
		printf("NV_4008 (0x4008): %08x\n", nv_card->PMC[0x4008/4]);
		printf("NV_400C (0x400c): %08x\n", nv_card->PMC[0x400c/4]);
		printf("NV_4010 (0x4010): %08x\n", nv_card->PMC[0x4010/4]);
		printf("NV_4014 (0x4014): %08x\n", nv_card->PMC[0x4014/4]);
		printf("NV_4018 (0x4018): %08x\n", nv_card->PMC[0x4018/4]);
		printf("NV_401C (0x401c): %08x\n", nv_card->PMC[0x401c/4]);
		printf("NV_4020 (0x4020): %08x\n", nv_card->PMC[0x4020/4]);
		printf("NV_4024 (0x4024): %08x\n", nv_card->PMC[0x4024/4]);
		printf("NV_4028 (0x4028): %08x\n", nv_card->PMC[0x4028/4]);
		printf("NV_402C (0x402c): %08x\n", nv_card->PMC[0x402c/4]);
		printf("NV_4030 (0x4030): %08x\n", nv_card->PMC[0x4030/4]);
		printf("NV_4034 (0x4034): %08x\n", nv_card->PMC[0x4034/4]);
		printf("NV_4038 (0x4038): %08x\n", nv_card->PMC[0x4038/4]);
		printf("NV_403C (0x403c): %08x\n", nv_card->PMC[0x403c/4]);
		printf("NV_4040 (0x4040): %08x\n", nv_card->PMC[0x4040/4]);
		printf("NV_4044 (0x4044): %08x\n", nv_card->PMC[0x4044/4]);
		printf("NV_4048 (0x4048): %08x\n", nv_card->PMC[0x4048/4]);
		printf("NV_404C (0x404c): %08x\n", nv_card->PMC[0x404c/4]);
		printf("NV_4050 (0x4050): %08x\n", nv_card->PMC[0x4050/4]);
		printf("NV_4054 (0x4054): %08x\n", nv_card->PMC[0x4054/4]);
		printf("NV_4058 (0x4058): %08x\n", nv_card->PMC[0x4058/4]);
		printf("NV_405C (0x405c): %08x\n", nv_card->PMC[0x405c/4]);
		printf("NV_4060 (0x4060): %08x\n", nv_card->PMC[0x4060/4]);
	}
	if(nv_card->arch & NV5X)
	{
		printf("NV_E100 (0xe100): %08x\n", nv_card->PMC[0xe100/4]);
		printf("NV_E114 (0xe114): %08x\n", nv_card->PMC[0xe114/4]);
		printf("NV_E118 (0xe118): %08x\n", nv_card->PMC[0xe118/4]);
		printf("NV_E11C (0xe11c): %08x\n", nv_card->PMC[0xe11c/4]);
		printf("NV_E120 (0xe120): %08x\n", nv_card->PMC[0xe120/4]);
		printf("NV_E300 (0xe300): %08x\n", nv_card->PMC[0xe300/4]);
		printf("NV_20008 (0x20008): %08x\n", nv_card->PMC[0x20008/4]);
		printf("NV_20400 (0x20400): %08x\n", nv_card->PMC[0x20400/4]);
		printf("NV_PDISPLAY_SOR0_REGS_BRIGHTNESS(%x): %08x\n", NV_PDISPLAY_SOR0_REGS_BRIGHTNESS, nv_card->PDISPLAY[NV_PDISPLAY_SOR0_REGS_BRIGHTNESS/4]);
	}

	printf("NV_PFB_CFG0 (0x100200): %08x\n", nv_card->PFB[0x200/4]);
	printf("NV_PFB_CFG0 (0x100204): %08x\n", nv_card->PFB[0x204/4]);
	printf("NV_PFB_CFG0 (0x100208): %08x\n", nv_card->PFB[0x208/4]);
	printf("NV_PFB_CFG0 (0x10020c): %08x\n", nv_card->PFB[0x20c/4]);
	printf("NV_PFB_218  (0x100218): %08x\n", nv_card->PFB[0x218/4]);
	printf("NV_PFB_TIMING0 (0x100220): %08x\n", nv_card->PFB[0x220/4]);
	printf("NV_PFB_TIMING1 (0x100224): %08x\n", nv_card->PFB[0x224/4]);
	printf("NV_PFB_TIMING2 (0x100228): %08x\n", nv_card->PFB[0x228/4]);
	printf("NV_PFB_474     (0x100474): %08x\n", nv_card->PFB[0x474/4]);
	printf("NV_PEXTDEV_BOOT_0 (0x101000): %08x\n", nv_card->PEXTDEV[0x0/4]);
	printf("NV_NVPLL_COEFF_A (0x680500): %08x\n", nv_card->PRAMDAC[0x500/4]);
	printf("NV_MPLL_COEFF_A (0x680504): %08x\n", nv_card->PRAMDAC[0x504/4]);
	printf("NV_VPLL_COEFF (0x680508): %08x\n", nv_card->PRAMDAC[0x508/4]);
	printf("NV_PLL_COEFF_SELECT (0x68050c): %08x\n", nv_card->PRAMDAC[0x50c/4]);
	printf("NV_NVPLL_COEFF_B (0x680570: %08x\n", nv_card->PRAMDAC[0x570/4]);
	printf("NV_MPLL_COEFF_B (0x680574: %08x\n", nv_card->PRAMDAC[0x574/4]);

	/* The builtin tvout encoder is available on Geforce4MX/TI and all other GPUs upto NV3x/NV4x.
	 * The registers are somewhere else on Geforce8 cards. There is a difference between the encoders
	 * on the difference cards but I'm not sure which apart from more features like the addition of
	 * component on the Geforce6 */
	if(nv_card->arch & (NV17 | NV25 | NV3X | NV4X))
	{
		int index=0;
		printf("--- TVOut regs ---\n");
		printf("0xd200: 0x%08x\n", nv_card->PMC[0xd200/4]); /* bit27-24 flickering (?) */
		printf("0xd204: 0x%08x\n", nv_card->PMC[0xd204/4]);
		printf("0xd208: 0x%08x\n", nv_card->PMC[0xd208/4]); /* Overscan */
		printf("0xd20c: 0x%08x\n", nv_card->PMC[0xd20c/4]);
		printf("0xd210: 0x%08x\n", nv_card->PMC[0xd210/4]); /* bit 23-8 contain the horizontal resolution */
		printf("0xd214: 0x%08x\n", nv_card->PMC[0xd214/4]); /* bit 23-8 contain the vertical resolution */
		printf("0xd218: 0x%08x\n", nv_card->PMC[0xd218/4]); /* bit31 = sign bit; bit16 and up can be used for horizontal positioning */

		printf("0xd21c: 0x%08x\n", nv_card->PMC[0xd21c/4]); /* bit31 = sign bit; bit16 and up can be used for vertical positioning */

		printf("0xd228: 0x%08x\n", nv_card->PMC[0xd228/4]);  /* is this some clock signal?? */
		printf("0xd22c: 0x%08x\n", nv_card->PMC[0xd22c/4]);
		
		printf("0xd230: 0x%08x\n", nv_card->PMC[0xd230/4]);
		printf("0xd304: 0x%08x\n", nv_card->PMC[0xd304/4]); /* bit 25-16 hscaler (PAL 720, NTSC 720) */
		printf("0xd508: 0x%08x\n", nv_card->PMC[0xd508/4]); /* bit 25-26 vscalar (PAL 288, NTSC 240) */
		printf("0xd600: 0x%08x\n", nv_card->PMC[0xd600/4]);
		printf("0xd604: 0x%08x\n", nv_card->PMC[0xd604/4]);
		printf("0xd608: 0x%08x\n", nv_card->PMC[0xd608/4]);

		/* Register 0xd220/0xd224 form a index/data register pair
		 * - 0x7 = bit4:0 connector type; bit2 s-video, bit2-0 empty: composite?
		 * - 0xe = bit7:0 tv system; ntscm (japan) 0x2; palb/d/g 0x4; palm/n/ntsc 0xc; is this correct? 
		 * - 0x22 = tv saturation
		 * - 0x25 = tv hue
		 * how many indices exist ?
		 */
		for(index=0; index < 0x80; index++)
		{
			nv_card->PMC[0xd220/4] = index;
			printf("index 0x%x: %02x\n", index, nv_card->PMC[0xd224/4]);
		}
	}
}


void info_init(void)
{
	nv_card->get_bus_type = get_bus_type;

	/* Set the pci id again as the detected id might not be accurate in case of pci id modding. The OS doesn't allways update the id while it really changed! Only do it for cards without bridges (device_id != 0xf* and 0x2e*
	* Don't use this for NV50 as the location of the pci config header has changed to an unknown position.
	*/
	if(((nv_card->device_id & 0xfff0) != 0xf0) && ((nv_card->device_id & 0xfff0) != 0x2e0) && !(nv_card->arch & NV5X))
	{
		nv_card->device_id = get_gpu_pci_id(); 
		nv_card->card_name = (char*)get_card_name(nv_card->device_id, &nv_card->gpu);
	}

	/* Set the pci subvendor id */
	nv_card->subvendor_id = get_pci_subvendor_id();

	/* gpu arch/revision */
	nv_card->get_gpu_architecture = get_gpu_architecture;
	nv_card->get_gpu_revision = get_gpu_revision;

	/* Allow modding on all Geforce cards except for ones using bridges */
	if((nv_card->arch & (NV1X | NV2X | NV3X | NV4X)) && ((nv_card->device_id & 0xfff0) != 0xf0))
	{
		nv_card->caps |= GPU_ID_MODDING;
		nv_card->set_gpu_pci_id = set_gpu_pci_id;
	}
	else
		nv_card->set_gpu_pci_id = NULL;

	/* Check if card is a native AGP one and not using a bridge chip else we can't use the code below */
	if(strcmp(nv_card->get_bus_type(), "AGP") == 0)
	{
		nv_card->get_bus_rate = get_agp_bus_rate;
		nv_card->get_agp_status = get_agp_status;
		nv_card->get_agp_fw_status = get_agp_fw_status;
		nv_card->get_agp_sba_status = get_agp_sba_status;
		nv_card->get_agp_supported_rates = get_agp_supported_rates;
	}
	/* Check if card is a native PCI-Express one and not using a bridge chip else we can't use the code below */
	else if(strcmp(nv_card->get_bus_type(), "PCI-Express") == 0)
	{
		nv_card->get_bus_rate = get_pcie_bus_rate;
		nv_card->get_pcie_max_bus_rate = get_pcie_max_bus_rate;
		nv_card->get_agp_status = NULL;
		nv_card->get_agp_fw_status = NULL;
		nv_card->get_agp_sba_status = NULL;
		nv_card->get_agp_supported_rates = NULL;
	}
	else
	{
		nv_card->get_bus_rate = NULL;
		nv_card->get_agp_status = NULL;
		nv_card->get_agp_fw_status = NULL;
		nv_card->get_agp_sba_status = NULL;
		nv_card->get_agp_supported_rates = NULL;
	}

	nv_card->get_memory_size = get_memory_size;
	nv_card->get_memory_type = get_memory_type;
	nv_card->get_memory_width = get_memory_width;

	/* Debugging stuff */
	nv_card->get_debug_info = get_debug_info;
}
