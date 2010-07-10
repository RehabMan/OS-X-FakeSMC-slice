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
 * Copyright (c) 1992 NeXT Computer, Inc.
 *
 * UFS byte swapping routines to make a big endian file system useful on a
 * little endian machine.
 *
 * HISTORY
 *
 * 8 Jul 1992 Brian Pinkerton at NeXT
 *      Created.
 */

#ifndef __LIBSAIO_UFS_BYTEORDER_H
#define __LIBSAIO_UFS_BYTEORDER_H

#include <sys/vnode.h>
#include <sys/buf.h>
#include <sys/disk.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/dinode.h>
#include <ufs/ufs/dir.h>
#include <ufs/ffs/fs.h>

void byte_swap_ints(unsigned int *array, int count);
void byte_swap_shorts(unsigned short *array, int count);
void byte_swap_longlongs(unsigned long long *array, int count);

void byte_swap_superblock(struct fs *sb);
void byte_swap_dinode_in(struct dinode *di);
void byte_swap_dir_block_in(char *addr, int count);
void byte_swap_inode_in(struct dinode *dc, struct dinode *ic);

#endif /* !__LIBSAIO_UFS_BYTEORDER_H */
