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

#include "libsaio.h"

/*  This NBP code is pretty useless because it just blindly calls INT 2B.
    Presumably INT 2B was implemented by some first-stage bootloader that
    is long gone.

    One good reason to disable this is that nbpScanBootVolumes always
    succeeds.  The scanBootVolumes function thus never fails because
    it always falls back to NBP.  This is a problem because there is
    other code in the booter (for example, in open) which needs to
    fail instead of attempting to use this NBP which will often
    hang the machine.
 */
#ifndef NBP_SUPPORT
#define NBP_SUPPORT 0
#endif

#if NBP_SUPPORT

/*
 * Convert zero-based linear address to far pointer.
 */
#define GET_FP(x)   ( (((x) & 0xffff0000) << (16 - 4)) | ((x) & 0xffff) )

/*==========================================================================
 * Issue a command to the network loader.
 *
 * The 'cmd' command structure should be allocated on the stack to
 * ensure that it resides within the addressable range for the
 * network loader, which runs in real mode.
 */
static UInt32 nbp(nbpCommandCode_t code, nbpCommand_u * cmd)
{
	loader(code, GET_FP((UInt32) cmd));

	// Must re-enable the A20 address line, the PXE firmware will
	// disable the A20 line control.
	//
	enableA20();

	return cmd->header.status;
}

/*==========================================================================
 * Unload Base Code Stack command.
 */
UInt32 nbpUnloadBaseCode()
{
    return nbp(nbpCommandUnloadBaseCode, (nbpCommand_u *) 0);
}

/*==========================================================================
 * TFTP Read File command.
 */
static long NBPLoadFile(CICell ih, char * filePath)
{
    nbpCommandTFTPReadFile_s  cmd;
	UInt32                    ret;

	strcpy((char *)cmd.filename, filePath);
	cmd.status     = nbpStatusFailed;
	cmd.bufferSize = TFTP_LEN;
	cmd.buffer     = TFTP_ADDR;

	verbose("Loading file: %s\n", filePath);

	ret = nbp(nbpCommandTFTPReadFile, (nbpCommand_u *) &cmd);

    return (ret == nbpStatusSuccess) ? (long)cmd.bufferSize : -1;
}

/*==========================================================================
 * GetDirEntry is not supported.
 */
static long NBPGetDirEntry(CICell ih, char * dirPath, long long * dirIndex,
                           char ** name, long * flags, long * time,
                           FinderInfo * finderInfo, long * infoValid)
{
    return -1;
}

//==========================================================================

static void NBPGetDescription(CICell ih, char * str, long strMaxLen)
{
    sprintf( str, "Ethernet PXE Client" );
}

//==========================================================================

BVRef nbpScanBootVolumes( int biosdev, int * countPtr )
{
    static BVRef gNetBVR = NULL;

    if ( countPtr ) *countPtr = 1;

    if ( !gNetBVR )
    {
        gNetBVR = malloc( sizeof(*gNetBVR) );
        if ( gNetBVR )
        {
            bzero(gNetBVR, sizeof(*gNetBVR));
            gNetBVR->biosdev = biosdev;
            gNetBVR->flags   = kBVFlagPrimary | kBVFlagNativeBoot;
            gNetBVR->description = NBPGetDescription;
            gNetBVR->fs_loadfile = NBPLoadFile;
            gNetBVR->fs_getdirentry = NBPGetDirEntry;
        }
    }
    return gNetBVR;
}
#else
BVRef nbpScanBootVolumes( int biosdev, int * countPtr )
{
    return NULL;
}
UInt32 nbpUnloadBaseCode()
{
    return 0;
}
#endif
