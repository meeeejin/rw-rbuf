/*****************************************************************************

Copyright (c) 1996, 2014, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA

*****************************************************************************/

/**************************************************//**
@file include/rw0rw.h
Data structures for the RW command

Created 3/15/2021 Anonymous author
*******************************************************/

#ifndef rw0rw_h
#define rw0rw_h

#define RW_CMD 1

extern unsigned long int n_lru_batch;
extern unsigned long int n_ckpt_batch;
extern unsigned long int n_rwcmd;
extern unsigned long int n_spf;
extern unsigned long int n_total_reads;

extern unsigned long int n_free_block;
extern unsigned long int n_bg_wait;
extern unsigned long int n_clean_block;
extern unsigned long int n_no_block;

#ifdef RW_CMD
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include "os0file.h"

typedef unsigned long int uintptr_t;
#define START_SECTOR 2048
extern int dev_fd;
extern int log_fd;

struct nvme_passthru_cmd {
  __u8  opcode;
  __u8  flags;
  __u16 rsvd1;
  __u32 nsid;
  __u32 cdw2;
  __u32 cdw3;
  __u64 metadata;
  __u64 addr;
  __u32 metadata_len;
  __u32 data_len;
  __u32 cdw10;
  __u32 cdw11;
  __u32 cdw12;
  __u32 cdw13;
  __u32 cdw14;
  __u32 cdw15;
  __u32 timeout_ms;
  __u32 result;
};

#define NVME_IOCTL_IO_CMD _IOWR('N', 0x43, struct nvme_passthru_cmd)

/*******************************************************************//**
Get LBA of the given block using fd and offset.
@ return LBA*/
UNIV_INTERN
ib_uint64_t
buf_get_lba(
/*========*/
	int file,	/*!< in: handle to a file */
	ib_uint64_t	offset);	/*!< in: file offset where to write */

/****************************************************************//**
Opens an device for the ioctl() function.
@return own: handle to the file, not defined if error */
UNIV_INTERN
int
os_file_open_dev(
/*=============*/
	const char*	name);	/*!< in: name of the device */

/****************************************************************//**
Closes a file handle.
@return	TRUE if success */
UNIV_INTERN
ibool
os_file_close_dev(
/*==============*/
	int	file);	/*!< in, own: handle to a file */
#endif /* RW_CMD */

#endif
