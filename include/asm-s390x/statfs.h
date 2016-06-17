/*
 *  include/asm-s390/statfs.h
 *
 *  S390 version
 *
 *  Derived from "include/asm-i386/statfs.h"
 */

#ifndef _S390_STATFS_H
#define _S390_STATFS_H

#ifndef __KERNEL_STRICT_NAMES

#include <linux/types.h>

typedef __kernel_fsid_t	fsid_t;

#endif

struct statfs {
	int  f_type;
	int  f_bsize;
	long f_blocks;
	long f_bfree;
	long f_bavail;
	long f_files;
	long f_ffree;
	__kernel_fsid_t f_fsid;
	int  f_namelen;
	int  f_spare[6];
};

#endif
