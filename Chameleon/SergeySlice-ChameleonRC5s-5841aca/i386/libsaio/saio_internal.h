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

#ifndef __LIBSAIO_SAIO_INTERNAL_H
#define __LIBSAIO_SAIO_INTERNAL_H

#include "saio_types.h"

/* asm.s */
extern void   real_to_prot(void);
extern void   prot_to_real(void);
extern void   halt(void);
extern void   startprog(unsigned int address, void *arg);
extern void   loader(UInt32 code, UInt32 cmdptr);

/* bios.s */
extern void   bios(biosBuf_t *bb);

/* biosfn.c */
#ifdef EISA_SUPPORT
extern bool   eisa_present(void);
#endif
extern int    bgetc(void);
extern int    biosread(int dev, int cyl, int head, int sec, int num);
extern int    ebiosread(int dev, unsigned long long sec, int count);
extern int    ebioswrite(int dev, long sec, int count);
extern int    get_drive_info(int drive, struct driveInfo *dp);
extern int    ebiosEjectMedia(int biosdev);
extern void   putc(int ch);
extern void   putca(int ch, int attr, int repeat);
extern int    getc(void);
extern void   pause();
extern int    readKeyboardStatus(void);
extern int    readKeyboardShiftFlags(void);
extern unsigned int time18(void);
extern void   delay(int ms);
extern unsigned int get_diskinfo(int dev);
#if APM_SUPPORT
extern int    APMPresent(void);
extern int    APMConnect32(void);
#endif
extern int    memsize(int i);
extern void   video_mode(int mode);
extern void   setCursorPosition(int x, int y, int page);
extern void   setCursorType(int type);
extern void   getCursorPositionAndType(int *x, int *y, int *type);
extern void   scollPage(int x1, int y1, int x2, int y2, int attr, int rows, int dir);
extern void   clearScreenRows(int y1, int y2);
extern void   setActiveDisplayPage( int page );
extern unsigned long getMemoryMap(struct MemoryRange * rangeArray, unsigned long maxRangeCount,
                                  unsigned long * conMemSizePtr, unsigned long * extMemSizePtr);
extern unsigned long getExtendedMemorySize();
extern unsigned long getConventionalMemorySize();
extern void   sleep(int n);

/* bootstruct.c */
extern void   initKernBootStruct(void);
extern void   reserveKernBootStruct(void);
extern void   copyKernBootStruct(void);
extern void   finalizeBootStruct(void);

/* cache.c */
extern void CacheReset();
extern void   CacheInit(CICell ih, long blockSize);
extern long   CacheRead(CICell ih, char *buffer, long long offset,
                        long length, long cache);

/* console.c */
extern bool   gVerboseMode;
extern bool   gErrors;
extern void   initBooterLog(void);
extern void   setupBooterLog(void);
extern void   putchar(int ch);
extern int    getchar(void);
extern void   msglog(const char * format, ...);
extern int    printf(const char *format, ...);
extern int    error(const char *format, ...);
extern int    verbose(const char *format, ...);
extern void   stop(const char *format, ...);

/* disk.c */
extern void rescanBIOSDevice(int biosdev);
extern struct DiskBVMap* diskResetBootVolumes(int biosdev);
extern void diskFreeMap(struct DiskBVMap *map);
extern int    testBiosread( int biosdev, unsigned long long secno );
extern BVRef  diskScanBootVolumes(int biosdev, int *count);
extern void   diskSeek(BVRef bvr, long long position);
extern int    diskRead(BVRef bvr, long addr, long length);
extern int    diskIsCDROM(BVRef bvr);
extern int    biosDevIsCDROM(int biosdev);
extern BVRef  getBVChainForBIOSDev(int biosdev);
extern BVRef  newFilteredBVChain(int minBIOSDev, int maxBIOSDev, unsigned int allowFlags, unsigned int denyFlags, int *count);
extern int    freeFilteredBVChain(const BVRef chain);
extern int    rawDiskRead(BVRef bvr, unsigned int secno, void *buffer, unsigned int len);
extern int    rawDiskWrite(BVRef bvr, unsigned int secno, void *buffer, unsigned int len);
extern int    readBootSector(int biosdev, unsigned int secno, void *buffer);
extern void   turnOffFloppy(void);
extern int	  testFAT32EFIBootSector( int biosdev, unsigned int secno, void * buffer );

/* hfs_compare.c */
extern int32_t FastUnicodeCompare(u_int16_t *uniStr1, u_int32_t len1,
							   u_int16_t *uniStr2, u_int32_t len2, int byte_order);
extern void utf_encodestr( const u_int16_t * ucsp, int ucslen,
                u_int8_t * utf8p, u_int32_t bufsize, int byte_order );
extern void utf_decodestr(const u_int8_t *utf8p, u_int16_t *ucsp,
                u_int16_t *ucslen, u_int32_t bufsize, int byte_order );

/* load.c */
extern bool   gHaveKernelCache;
extern long ThinFatFile(void **binary, unsigned long *length);
extern long DecodeMachO(void *binary, entry_t *rentry, char **raddr, int *rsize);

/* memory.c */
long AllocateKernelMemory( long inSize );
long AllocateMemoryRange(char * rangeName, long start, long length, long type);

/* misc.c */
extern void   enableA20(void);
extern int    checkForSupportedHardware();
extern int	  isLaptop();
extern void   getPlatformName(char *nameBuf);

/* nbp.c */
extern UInt32 nbpUnloadBaseCode();
extern BVRef  nbpScanBootVolumes(int biosdev, int *count);

/* stringTable.c */
extern char * newStringFromList(char **list, int *size);
extern int    stringLength(const char *table, int compress);
extern bool   getValueForConfigTableKey(config_file_t *config, const char *key, const char **val, int *size);
extern bool   removeKeyFromTable(const char *key, char *table);
extern char * newStringForStringTableKey(config_file_t *config, char *key);
extern char * newStringForKey(char *key, config_file_t *configBuff);
extern bool   getValueForBootKey(const char *line, const char *match, const char **matchval, int *len);
extern bool   getValueForKey(const char *key, const char **val, int *size, config_file_t *configBuff);
extern const char * getStringForKey(const char * key,  config_file_t *config);
extern bool   getBoolForKey(const char *key, bool *val, config_file_t *configBuff);
extern bool   getIntForKey(const char *key, int *val, config_file_t *configBuff);
extern bool   getColorForKey(const char *key, unsigned int *val, config_file_t *configBuff);
extern bool	  getDimensionForKey( const char *key, unsigned int *value, config_file_t *config, unsigned int dimension_max, unsigned int object_size );
extern int    loadConfigFile(const char *configFile, config_file_t *configBuff);
extern int    loadSystemConfig(config_file_t *configBuff);
extern int    loadHelperConfig(config_file_t *configBuff);
extern int    loadOverrideConfig(config_file_t *configBuff);
extern char * newString(const char *oldString);
extern char * getNextArg(char ** ptr, char * val);
extern int	  ParseXMLFile( char * buffer, TagPtr * dict );

/* sys.c */
extern BVRef getBootVolumeRef( const char * path, const char ** outPath );
extern long   LoadVolumeFile(BVRef bvr, const char *fileSpec);
extern long   LoadFile(const char *fileSpec);
extern long   ReadFileAtOffset(const char * fileSpec, void *buffer, uint64_t offset, uint64_t length);
extern long   LoadThinFatFile(const char *fileSpec, void **binary);
extern long   GetDirEntry(const char *dirSpec, long long *dirIndex, const char **name,
                          long *flags, long *time);
extern long   GetFileInfo(const char *dirSpec, const char *name,
                          long *flags, long *time);
extern long   GetFileBlock(const char *fileSpec, unsigned long long *firstBlock);
extern long   GetFSUUID(char *spec, char *uuidStr);
extern long   CreateUUIDString(uint8_t uubytes[], int nbytes, char *uuidStr);
extern int    openmem(char *buf, int len);
extern int    open(const char *str, int how);
extern int    open_bvdev(const char *bvd, const char *path, int flags);
extern int    close(int fdesc);
extern int    file_size(int fdesc);
extern int    read(int fdesc, char *buf, int count);
extern int    write(int fdesc, const char *buf, int count);
extern int    writebyte(int fdesc, char value);
extern int    writeint(int fdesc, int value);
extern int    b_lseek(int fdesc, int addr, int ptr);
extern int    tell(int fdesc);
extern const char * systemConfigDir(void);
extern struct dirstuff * opendir(const char *path);
extern struct dirstuff * vol_opendir(BVRef bvr, const char *path);
extern int    closedir(struct dirstuff *dirp);
extern int    readdir(struct dirstuff *dirp, const char **name, long *flags, long *time);
extern int    readdir_ext(struct dirstuff * dirp, const char ** name, long * flags,
                          long * time, FinderInfo *finderInfo, long *infoValid);
extern void   flushdev(void);
extern void   scanBootVolumes(int biosdev, int *count);
extern void   scanDisks(int biosdev, int *count);
extern BVRef  selectBootVolume(BVRef chain);
extern void   getBootVolumeDescription(BVRef bvr, char *str, long strMaxLen, bool verbose);
extern void   setRootVolume(BVRef volume);
extern void   setBootGlobals(BVRef chain);
extern int    getDeviceDescription(BVRef volume, char *str);

extern int    gBIOSDev;
extern int    gBootFileType;
extern BVRef  gBootVolume;
extern BVRef  gBIOSBootVolume;

// Function pointer to be filled in if ramdisks are available
extern int (*p_get_ramdisk_info)(int biosdev, struct driveInfo *dip);
extern int (*p_ramdiskReadBytes)( int biosdev, unsigned int blkno,
                      unsigned int byteoff,
                      unsigned int byteCount, void * buffer );

#endif /* !__LIBSAIO_SAIO_INTERNAL_H */
