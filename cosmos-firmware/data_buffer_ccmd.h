#ifndef SRC_DATA_BUFFER_CCMD_H_
#define SRC_DATA_BUFFER_CCMD_H_

#include "ftl_config.h"

#define AVAILABLE_CCMD_DATA_BUFFER_ENTRY_COUNT				(1 * USER_DIES)

#define CCMD_DATA_BUF_NONE	0xffff
#define CCMD_DATA_BUF_FAIL	0xffff
#define CCMD_DATA_BUF_DIRTY	1
#define CCMD_DATA_BUF_CLEAN	0

#define FindCcmdDataBufHashTableEntry(logicalSliceAddr) ((logicalSliceAddr) % AVAILABLE_CCMD_DATA_BUFFER_ENTRY_COUNT)

typedef struct _CCMD_DATA_BUF_ENTRY {
	unsigned int logicalSliceAddr;
	unsigned int prevEntry : 16;
	unsigned int nextEntry : 16;
	unsigned int blockingReqTail : 16;
	unsigned int hashPrevEntry : 16;
	unsigned int hashNextEntry : 16;
	unsigned int dirty : 1;
	unsigned int reserved0 : 15;
} CCMD_DATA_BUF_ENTRY, *P_CCMD_DATA_BUF_ENTRY;

typedef struct _CCMD_DATA_BUF_MAP{
	CCMD_DATA_BUF_ENTRY dataBuf[AVAILABLE_DATA_BUFFER_ENTRY_COUNT];
} CCMD_DATA_BUF_MAP, *P_CCMD_DATA_BUF_MAP;

typedef struct _CCMD_DATA_BUF_LRU_LIST {
	unsigned int headEntry : 16;
	unsigned int tailEntry : 16;
} CCMD_DATA_BUF_LRU_LIST, *P_CCMD_DATA_BUF_LRU_LIST;

typedef struct _CCMD_DATA_BUF_HASH_ENTRY{
	unsigned int headEntry : 16;
	unsigned int tailEntry : 16;
} CCMD_DATA_BUF_HASH_ENTRY, *P_CCMD_DATA_BUF_HASH_ENTRY;


typedef struct _CCMD_DATA_BUF_HASH_TABLE{
	CCMD_DATA_BUF_HASH_ENTRY dataBufHash[AVAILABLE_DATA_BUFFER_ENTRY_COUNT];
} CCMD_DATA_BUF_HASH_TABLE, *P_CCMD_DATA_BUF_HASH_TABLE;


void InitCcmdDataBuf();
unsigned int CheckCcmdDataBufHit(unsigned int reqSlotTag);
unsigned int AllocateCcmdDataBuf();
void UpdateCcmdDataBufEntryInfoBlockingReq(unsigned int bufEntry, unsigned int reqSlotTag);

void PutToCcmdDataBufHashList(unsigned int bufEntry);
void SelectiveGetFromCcmdDataBufHashList(unsigned int bufEntry);

extern P_CCMD_DATA_BUF_MAP ccmdDataBufMapPtr;
extern CCMD_DATA_BUF_LRU_LIST ccmdDataBufLruList;
extern P_CCMD_DATA_BUF_HASH_TABLE ccmdDataBufHashTable;


#endif /* SRC_DATA_BUFFER_CCMD_H_ */
