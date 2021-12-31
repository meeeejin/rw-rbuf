/**************************************************//**
@file rw/rw0rw.cc
The Read/Write command to solve the RAW problem

Created 4/2/2021 Anonymous author
*******************************************************/

#include "rw0rw.h"

unsigned long int n_lru_batch = 0;
unsigned long int n_ckpt_batch = 0;
unsigned long int n_spf = 0;
unsigned long int n_rwcmd = 0;
unsigned long int n_total_reads = 0;

unsigned long int n_free_block = 0;
unsigned long int n_bg_wait = 0;
unsigned long int n_clean_block = 0;
unsigned long int n_no_block = 0;

#ifdef RW_CMD
int dev_fd;
int log_fd;

/*******************************************************************//**
Get LBA of the given block using fd and offset.
@ return LBA*/
UNIV_INTERN
ib_uint64_t
buf_get_lba(
/*========*/
	int file,	/*!< in: handle to a file */
	ib_uint64_t	offset)	/*!< in: file offset where to write */
{
  ib_uint64_t ret;
 	ib_uint64_t block;
  struct stat st;

  fstat(file, &st);

  block = st.st_blksize;

  /* Get page number */
  ret = offset / block;

	/* Get LBA using FIBMAP */
  if (ioctl(file, FIBMAP, &ret) < 0) {
    perror("Error while getting LBA");
		fprintf(stderr, "%d\n", file);
		ut_error;
  }

	/* Transform the LBA to fit the NVMe spec */
  ret = ((ib_uint64_t)ret * 8 + START_SECTOR) / 8;

  return(ret);
}

/****************************************************************//**
Open a device for the ioctl() function.
@return own: handle to the file, not defined if error */
UNIV_INTERN
int
os_file_open_dev(
/*=============*/
	const char*	name)	/*!< in: name of the device */
{
	int file;
	
	file = open(name, O_DIRECT | O_RDWR); 

	if (file == -1) {
		perror("Error while opening the device");
		fprintf(stderr, "%s\n", name);
		ut_error;
	}

	/*char cmd_buf[1024];
	sprintf(cmd_buf, "sudo nvme vuc %s enable", name);
	system(cmd_buf);*/

	return(file);
}

/****************************************************************//**
Close a file handle.
@return	TRUE if success */
UNIV_INTERN
ibool
os_file_close_dev(
/*==============*/
	int	file)	/*!< in, own: handle to a file */
{
	int	ret;

	ret = close(file);

	if (ret == -1) {
		perror("Error while closing the device");
		ut_error;
	}

	return(TRUE);
}

#endif /* RW_CMD */
