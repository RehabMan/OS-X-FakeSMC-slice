/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*	$NetBSD: ntfs.h,v 1.9 1999/10/31 19:45:26 jdolecek Exp $	*/

/*-
 * Copyright (c) 1998, 1999 Semen Ustimenko
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/fs/ntfs/ntfs.h,v 1.14 2001/11/27 00:18:33 jhb Exp $
 */

/*#define NTFS_DEBUG 1*/

#ifdef APPLE
/* We're using FreeBSD style byte order macros in the source. */
#include <libkern/OSByteOrder.h>
#define le16toh(x)		OSSwapLittleToHostInt16(x)
#define le32toh(x)		OSSwapLittleToHostInt32(x)
#define le64toh(x)		OSSwapLittleToHostInt64(x)

/* FreeBSD mutexes correspond to Darwin's simple locks */
#define mtx_lock(lock)		simple_lock(lock)
#define mtx_unlock(lock)	simple_unlock(lock)
#define mtx_destroy(lock)	/* Nothing. */

#define lockdestroy(lock)	/* Nothing. */

#endif

typedef u_int64_t cn_t;
typedef u_int16_t wchar;

#pragma pack(1)
#define BBSIZE			1024
#define	BBOFF			((off_t)(0))
#define	BBLOCK			((daddr_t)(0))
#define	NTFS_MFTINO		0
#define	NTFS_VOLUMEINO		3
#define	NTFS_ATTRDEFINO		4
#define	NTFS_ROOTINO		5
#define	NTFS_BITMAPINO		6
#define	NTFS_BOOTINO		7
#define	NTFS_BADCLUSINO		8
#define	NTFS_UPCASEINO		10
#define NTFS_MAXFILENAME	255

struct fixuphdr {
	u_int32_t       fh_magic;
	u_int16_t       fh_foff;
	u_int16_t       fh_fnum;
};

#define NTFS_AF_INRUN	0x00000001
struct attrhdr {
	u_int32_t       a_type;
	u_int32_t       reclen;
	u_int8_t        a_flag;
	u_int8_t        a_namelen;
	u_int8_t        a_nameoff;
	u_int8_t        reserved1;
	u_int8_t        a_compression;
	u_int8_t        reserved2;
	u_int16_t       a_index;
};
#define NTFS_A_STD	0x10
#define NTFS_A_ATTRLIST	0x20
#define NTFS_A_NAME	0x30
#define NTFS_A_VOLUMENAME	0x60
#define NTFS_A_DATA	0x80
#define	NTFS_A_INDXROOT	0x90
#define	NTFS_A_INDX	0xA0
#define NTFS_A_INDXBITMAP 0xB0

#define NTFS_MAXATTRNAME	255
struct attr {
	struct attrhdr  a_hdr;
	union {
		struct {
			u_int16_t       a_datalen;
			u_int16_t       reserved1;
			u_int16_t       a_dataoff;
			u_int16_t       a_indexed;
		}               a_S_r;
		struct {
			cn_t            a_vcnstart;
			cn_t            a_vcnend;
			u_int16_t       a_dataoff;
			u_int16_t       a_compressalg;
			u_int32_t       reserved1;
			u_int64_t       a_allocated;
			u_int64_t       a_datalen;
			u_int64_t       a_initialized;
		}               a_S_nr;
	}               a_S;
};
#define a_r	a_S.a_S_r
#define a_nr	a_S.a_S_nr

typedef struct {
	u_int64_t       t_create;
	u_int64_t       t_write;
	u_int64_t       t_mftwrite;
	u_int64_t       t_access;
}               ntfs_times_t;

#define NTFS_FFLAG_RDONLY	0x01LL
#define NTFS_FFLAG_HIDDEN	0x02LL
#define NTFS_FFLAG_SYSTEM	0x04LL
#define NTFS_FFLAG_ARCHIVE	0x20LL
#define NTFS_FFLAG_COMPRESSED	0x0800LL
#define NTFS_FFLAG_DIR		0x10000000LL

struct attr_name {
	u_int32_t	n_pnumber;	/* Parent ntnode */
	u_int32_t       reserved;
	ntfs_times_t    n_times;
	u_int64_t       n_size;
	u_int64_t       n_attrsz;
	u_int64_t       n_flag;
	u_int8_t        n_namelen;
	u_int8_t        n_nametype;
	u_int16_t       n_name[1];
};

#define NTFS_IRFLAG_INDXALLOC	0x00000001
struct attr_indexroot {
	u_int32_t       ir_unkn1;	/* attribute type (0x30 for $FILE_NAME) */
	u_int32_t       ir_unkn2;	/* collation rule (0x01 for file names) */
	u_int32_t       ir_size;	/* size of index allocation entry */
	u_int32_t       ir_unkn3;	/* clusters per index record, and 3 bytes padding */
	u_int32_t       ir_unkn4;	/* (offset to first index entry?) always 0x10 */
	u_int32_t       ir_datalen;	/* (total size of index enties?) sizeof something */
	u_int32_t       ir_allocated;	/* (allocated size of index entries?) */
	u_int8_t       	ir_flag;	/* 1=index allocation needed (large index) */
        u_int8_t	ir_pad1;	/* padding */
	u_int16_t       ir_pad2;	/* padding */
};

struct attr_attrlist {
	u_int32_t       al_type;	/* Attribute type */
	u_int16_t       reclen;		/* length of this entry */
	u_int8_t        al_namelen;	/* Attribute name len */
	u_int8_t        al_nameoff;	/* Name offset from entry start */
	u_int64_t       al_vcnstart;	/* VCN number */
	u_int32_t       al_inumber;	/* Parent ntnode */
	u_int32_t       reserved;
	u_int16_t       al_index;	/* Attribute index in MFT record */
	u_int16_t       al_name[1];	/* Name */
};

#define	NTFS_INDXMAGIC	(u_int32_t)(0x58444E49)
struct attr_indexalloc {
	struct fixuphdr ia_fixup;
	u_int64_t       unknown1;
	cn_t            ia_bufcn;
	u_int16_t       ia_hdrsize;
	u_int16_t       unknown2;
	u_int32_t       ia_inuse;
	u_int32_t       ia_allocated;
};

#define	NTFS_IEFLAG_SUBNODE	0x00000001
#define	NTFS_IEFLAG_LAST	0x00000002

struct attr_indexentry {
	u_int32_t       ie_number;
	u_int32_t       unknown1;
	u_int16_t       reclen;
	u_int16_t       ie_size;
	u_int32_t       ie_flag;/* 1 - has subnodes, 2 - last */
	u_int32_t       ie_fpnumber;
	u_int32_t       unknown2;
	ntfs_times_t    ie_ftimes;
	u_int64_t       ie_fallocated;
	u_int64_t       ie_fsize;
	u_int32_t       ie_fflag;
        u_int32_t	unknown3;	/* used by reparse points and external attributes? */
	u_int8_t        ie_fnamelen;
	u_int8_t        ie_fnametype;
	wchar           ie_fname[NTFS_MAXFILENAME];
	/* cn_t		ie_bufcn;	 buffer with subnodes */
};

#define	NTFS_FILEMAGIC	(u_int32_t)(0x454C4946)
#define	NTFS_FRFLAG_DIR	0x0002
struct filerec {
	struct fixuphdr fr_fixup;
	u_int8_t        reserved[8];
	u_int16_t       fr_seqnum;	/* Sequence number */
	u_int16_t       fr_nlink;
	u_int16_t       fr_attroff;	/* offset to attributes */
	u_int16_t       fr_flags;	/* 1-nonresident attr, 2-directory */
	u_int32_t       fr_size;/* hdr + attributes */
	u_int32_t       fr_allocated;	/* allocated length of record */
	u_int64_t       fr_mainrec;	/* main record */
	u_int16_t       fr_attrnum;	/* maximum attr number + 1 ??? */
};

#define	NTFS_ATTRNAME_MAXLEN	0x40
#define	NTFS_ADFLAG_NONRES	0x0080	/* Attrib can be non resident */
#define	NTFS_ADFLAG_INDEX	0x0002	/* Attrib can be indexed */
struct attrdef {
	wchar		ad_name[NTFS_ATTRNAME_MAXLEN];
	u_int32_t	ad_type;
	u_int32_t	reserved1[2];
	u_int32_t	ad_flag;
	u_int64_t	ad_minlen;
	u_int64_t	ad_maxlen;	/* -1 for nonlimited */
};

struct ntvattrdef {
	char		ad_name[0x40];
	int		ad_namelen;
	u_int32_t	ad_type;
};

#define	NTFS_BBID	"NTFS    "
#define	NTFS_BBIDLEN	8
struct bootfile {
	u_int8_t        reserved1[3];	/* asm jmp near ... */
	u_int8_t        bf_sysid[8];	/* 'NTFS    ' */
	u_int16_t       bf_bps;		/* bytes per sector */
	u_int8_t        bf_spc;		/* sectors per cluster */
	u_int8_t        reserved2[7];	/* unused (zeroed) */
	u_int8_t        bf_media;	/* media desc. (0xF8) */
	u_int8_t        reserved3[2];
	u_int16_t       bf_spt;		/* sectors per track */
	u_int16_t       bf_heads;	/* number of heads */
	u_int8_t        reserver4[12];
	u_int64_t       bf_spv;		/* sectors per volume */
	cn_t            bf_mftcn;	/* $MFT cluster number */
	cn_t            bf_mftmirrcn;	/* $MFTMirr cn */
	u_int8_t        bf_mftrecsz;	/* MFT record size (clust) */
					/* 0xF6 inducates 1/4 */
	u_int32_t       bf_ibsz;	/* index buffer size */
	u_int32_t       bf_volsn;	/* volume ser. num. */
};

/*
 * Darwin's ntfs.util needs to include this file to get definitions
 * for the on-disk structures.  It doesn't need the ntfsmount structure.
 * In fact, since it doesn't #define KERNEL, the struct netexport below
 * won't be defined.
 *
 * So, I'm using #ifdef KERNEL around the things that are only relevant
 * to the in-kernel implementation.
 *
 *�� I don't know if FreeBSD defines KERNEL, or if I need to use or
 * invent a different conditional here.
 */
#ifdef KERNEL

#define	NTFS_SYSNODESNUM	0x0B
struct ntfsmount {
	struct mount   *ntm_mountp;	/* filesystem vfs structure */
	struct bootfile ntm_bootfile;
	dev_t           ntm_dev;	/* device mounted */
	struct vnode   *ntm_devvp;	/* block device mounted vnode */
	struct vnode   *ntm_sysvn[NTFS_SYSNODESNUM];
	u_int32_t       ntm_bpmftrec;
	uid_t           ntm_uid;
	gid_t           ntm_gid;
	mode_t          ntm_mode;
	u_long          ntm_flag;
	cn_t		ntm_cfree;
	struct ntvattrdef *ntm_ad;	/* attribute names are stored in native byte order */
	int		ntm_adnum;
 	wchar *		ntm_82u;	/* 8bit to Unicode */
 	char **		ntm_u28;	/* Unicode to 8 bit */
#ifdef APPLE
	struct netexport ntm_export;	/* NFS export information */
#endif
};

#define ntm_mftcn	ntm_bootfile.bf_mftcn
#define ntm_mftmirrcn	ntm_bootfile.bf_mftmirrcn
#define	ntm_mftrecsz	ntm_bootfile.bf_mftrecsz
#define	ntm_spc		ntm_bootfile.bf_spc
#define	ntm_bps		ntm_bootfile.bf_bps

#pragma pack()

#define	NTFS_NEXTREC(s, type) ((type)(((caddr_t) s) + le16toh((s)->reclen)))

/* Convert mount ptr to ntfsmount ptr. */
#define VFSTONTFS(mp)	((struct ntfsmount *)((mp)->mnt_data))
#define VTONT(v)	FTONT(VTOF(v))
#define	VTOF(v)		((struct fnode *)((v)->v_data))
#define	FTOV(f)		((f)->f_vp)
#define	FTONT(f)	((f)->f_ip)
#define ntfs_cntobn(cn)	((daddr_t)(cn) * (ntmp->ntm_spc))
#define ntfs_cntob(cn)	((off_t)(cn) * (ntmp)->ntm_spc * (ntmp)->ntm_bps)
#define ntfs_btocn(off)	(cn_t)((off) / ((ntmp)->ntm_spc * (ntmp)->ntm_bps))
#define ntfs_btocl(off)	(cn_t)((off + ntfs_cntob(1) - 1) / ((ntmp)->ntm_spc * (ntmp)->ntm_bps))
#define ntfs_btocnoff(off)	(off_t)((off) % ((ntmp)->ntm_spc * (ntmp)->ntm_bps))
#define ntfs_bntob(bn)	(daddr_t)((bn) * (ntmp)->ntm_bps)

#define	ntfs_bpbl	(daddr_t)((ntmp)->ntm_bps)

#ifdef MALLOC_DECLARE
MALLOC_DECLARE(M_NTFSMNT);
MALLOC_DECLARE(M_NTFSNTNODE);
MALLOC_DECLARE(M_NTFSFNODE);
MALLOC_DECLARE(M_NTFSDIR);
MALLOC_DECLARE(M_NTFSNTHASH);
#endif

#ifndef M_NTFSMNT
#define M_NTFSMNT M_TEMP
#endif
#ifndef M_NTFSNTNODE
#define M_NTFSNTNODE M_TEMP
#endif
#ifndef M_NTFSFNODE
#define M_NTFSFNODE M_TEMP
#endif
#ifndef M_NTFSDIR
#define M_NTFSDIR M_TEMP
#endif
#ifndef M_NTFSNTHASH
#define M_NTFSNTHASH M_TEMP
#endif
#ifndef M_NTFSRUN
#define M_NTFSRUN M_TEMP
#endif
#ifndef M_NTFSRDATA
#define M_NTFSRDATA M_TEMP
#endif
#ifndef M_NTFSNTVATTR
#define M_NTFSNTVATTR M_TEMP
#endif
#ifndef M_NTFSDECOMP
#define M_NTFSDECOMP M_TEMP
#endif
#define VT_NTFS VT_OTHER

#if defined(NTFS_DEBUG)
#define dprintf(a) printf a
#if NTFS_DEBUG > 1
#define ddprintf(a) printf a
#else
#define ddprintf(a)
#endif
#else
#define dprintf(a)
#define ddprintf(a)
#endif

#ifdef APPLE
typedef int     vop_t(void *);
#else
#endif
extern vop_t  **ntfs_vnodeop_p;
#endif /* KERNEL */
