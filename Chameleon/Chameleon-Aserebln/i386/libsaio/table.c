/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* 
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 * 			INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license  agreement or 
 *	nondisclosure agreement with Intel Corporation and may not be copied 
 *	nor disclosed except in accordance with the terms of that agreement.
 *
 *	Copyright 1988, 1989 Intel Corporation
 */

/*
 * Copyright 1993 NeXT, Inc.
 * All rights reserved.
 */
 
#include "memory.h"

/*  Segment Descriptor
 *
 * 31          24         19   16                 7           0
 * ------------------------------------------------------------
 * |             | |B| |A|       | |   |1|0|E|W|A|            |
 * | BASE 31..24 |G|/|0|V| LIMIT |P|DPL|  TYPE   | BASE 23:16 |
 * |             | |D| |L| 19..16| |   |1|1|C|R|A|            |
 * ------------------------------------------------------------
 * |                             |                            |
 * |        BASE 15..0           |       LIMIT 15..0          |
 * |                             |                            |
 * ------------------------------------------------------------
 */

struct seg_desc {
    unsigned short  limit_15_0;
    unsigned short  base_15_0;
    unsigned char   base_23_16;
    unsigned char   bit_15_8;
    unsigned char   bit_23_16;
    unsigned char   base_31_24;
};

// turbo - GDT must be in first 64k segment
struct seg_desc __attribute__ ((section("__INIT,__data"))) Gdt[ NGDTENT ] = {
    /*  0x0 : null */
    {0x0000, 0x0000,  0x00, 0x00, 0x00, 0x00},

    /*  0x8 : boot protected mode 32-bit code segment
              byte granularity, 1MB limit, MEMBASE offset */
    {0xFFFF, MEMBASE, 0x00, 0x9E, 0x4F, 0x00},    
    
    /* 0x10 : boot protected mode data segment
              page granularity, 4GB limit, MEMBASE offset */
    {0xFFFF, MEMBASE, 0x00, 0x92, 0xCF, 0x00},

    /* 0x18 : boot protected mode 16-bit code segment
              byte granularity, 1MB limit, MEMBASE offset */
    {0xFFFF, MEMBASE, 0x00, 0x9E, 0x0F, 0x00},

    /* 0x20 : kernel init 32-bit data segment
              page granularity, 4GB limit, zero offset */
    {0xFFFF, 0x0000,  0x00, 0x92, 0xCF, 0x00},

    /* 0x28 : kernel init 32-bit code segment
              page granularity, 4GB limit, zero offset */
    {0xFFFF, 0x0000,  0x00, 0x9E, 0xCF, 0x00},

    /* 0x30 : boot real mode data/stack segment
              byte granularity, 64K limit, MEMBASE offset, expand-up */
    {0xFFFF, MEMBASE, 0x00, 0x92, 0x00, 0x00},    
};
