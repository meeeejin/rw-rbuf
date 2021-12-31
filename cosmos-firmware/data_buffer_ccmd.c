#include "xil_printf.h"
#include <assert.h>
#include "memory_map.h"

P_CCMD_DATA_BUF_MAP ccmdDataBufMapPtr;
CCMD_DATA_BUF_LRU_LIST ccmdDataBufLruList;
P_CCMD_DATA_BUF_HASH_TABLE ccmdDataBufHashTablePtr;

void InitCcmdDataBuf()
{
	int bufEntry;

	ccmdDataBufMapPtr = (P_CCMD_DATA_BUF_MAP) CCMD_DATA_BUF_MAP_ADDR;
	ccmdDataBufHashTablePtr = (P_CCMD_DATA_BUF_HASH_TABLE)CCMD_DATA_BUF_HASH_TABLE_ADDR;

	for(bufEntry = 0; bufEntry < AVAILABLE_CCMD_DATA_BUFFER_ENTRY_COUNT; bufEntry++)
	{
		ccmdDataBufMapPtr->dataBuf[bufEntry].logicalSliceAddr = LSA_NONE;
		ccmdDataBufMapPtr->dataBuf[bufEntry].prevEntry = bufEntry-1;
		ccmdDataBufMapPtr->dataBuf[bufEntry].nextEntry = bufEntry+1;
		ccmdDataBufMapPtr->dataBuf[bufEntry].dirty = DATA_BUF_CLEAN;
		ccmdDataBufMapPtr->dataBuf[bufEntry].blockingReqTail =  REQ_SLOT_TAG_NONE;

		ccmdDataBufHashTablePtr->dataBufHash[bufEntry].headEntry = DATA_BUF_NONE;
		ccmdDataBufHashTablePtr->dataBufHash[bufEntry].tailEntry = DATA_BUF_NONE;
		ccmdDataBufMapPtr->dataBuf[bufEntry].hashPrevEntry = DATA_BUF_NONE;
		ccmdDataBufMapPtr->dataBuf[bufEntry].hashNextEntry = DATA_BUF_NONE;
	}

	ccmdDataBufMapPtr->dataBuf[0].prevEntry = DATA_BUF_NONE;
	ccmdDataBufMapPtr->dataBuf[AVAILABLE_CCMD_DATA_BUFFER_ENTRY_COUNT - 1].nextEntry = DATA_BUF_NONE;
	ccmdDataBufLruList.headEntry = 0 ;
	ccmdDataBufLruList.tailEntry = AVAILABLE_CCMD_DATA_BUFFER_ENTRY_COUNT - 1;
}
unsigned int CheckCcmdDataBufHit(unsigned int reqSlotTag)
{
	unsigned int bufEntry, logicalSliceAddr, prevBufEntry, nextBufEntry;

	logicalSliceAddr = reqPoolPtr->reqPool[reqSlotTag].logicalSliceAddr;
	bufEntry = ccmdDataBufHashTablePtr->dataBufHash[FindCcmdDataBufHashTableEntry(logicalSliceAddr)].headEntry;

	while(bufEntry != DATA_BUF_NONE)
	{
		if(ccmdDataBufMapPtr->dataBuf[bufEntry].logicalSliceAddr == logicalSliceAddr)
		{
			prevBufEntry = ccmdDataBufMapPtr->dataBuf[bufEntry].prevEntry;
			nextBufEntry = ccmdDataBufMapPtr->dataBuf[bufEntry].nextEntry;
			if((nextBufEntry != DATA_BUF_NONE) && (prevBufEntry != DATA_BUF_NONE))
			{
				ccmdDataBufMapPtr->dataBuf[prevBufEntry].nextEntry = ccmdDataBufMapPtr->dataBuf[bufEntry].nextEntry;
				ccmdDataBufMapPtr->dataBuf[nextBufEntry].prevEntry = ccmdDataBufMapPtr->dataBuf[bufEntry].prevEntry;
			}
			else if((nextBufEntry == DATA_BUF_NONE) && (prevBufEntry != DATA_BUF_NONE))
			{
				ccmdDataBufMapPtr->dataBuf[prevBufEntry].nextEntry = DATA_BUF_NONE;
				ccmdDataBufLruList.tailEntry = ccmdDataBufMapPtr->dataBuf[bufEntry].prevEntry;
			}
			else if((nextBufEntry != DATA_BUF_NONE) && (prevBufEntry == DATA_BUF_NONE))
			{
				ccmdDataBufMapPtr->dataBuf[nextBufEntry].prevEntry  = DATA_BUF_NONE;
				ccmdDataBufLruList.headEntry = ccmdDataBufMapPtr->dataBuf[bufEntry].nextEntry;
			}
			else
			{
				ccmdDataBufLruList.tailEntry = DATA_BUF_NONE;
				ccmdDataBufLruList.headEntry = DATA_BUF_NONE;
			}

			if(ccmdDataBufLruList.headEntry != DATA_BUF_NONE)
			{
				ccmdDataBufMapPtr->dataBuf[bufEntry].prevEntry = DATA_BUF_NONE;
				ccmdDataBufMapPtr->dataBuf[bufEntry].nextEntry = ccmdDataBufLruList.headEntry;
				ccmdDataBufMapPtr->dataBuf[ccmdDataBufLruList.headEntry].prevEntry = bufEntry;
				ccmdDataBufLruList.headEntry = bufEntry;
			}
			else
			{
				ccmdDataBufMapPtr->dataBuf[bufEntry].prevEntry = DATA_BUF_NONE;
				ccmdDataBufMapPtr->dataBuf[bufEntry].nextEntry = DATA_BUF_NONE;
				ccmdDataBufLruList.headEntry = bufEntry;
				ccmdDataBufLruList.tailEntry = bufEntry;
			}

			return bufEntry;
		}
		else
			bufEntry = ccmdDataBufMapPtr->dataBuf[bufEntry].hashNextEntry;
	}

	return DATA_BUF_FAIL;
}
unsigned int AllocateCcmdDataBuf()
{
	unsigned int evictedEntry = ccmdDataBufLruList.tailEntry;

	if(evictedEntry == DATA_BUF_NONE)
		assert(!"[WARNING] There is no valid buffer entry [WARNING]");

	if(ccmdDataBufMapPtr->dataBuf[evictedEntry].prevEntry != DATA_BUF_NONE)
	{
		ccmdDataBufMapPtr->dataBuf[ccmdDataBufMapPtr->dataBuf[evictedEntry].prevEntry].nextEntry = DATA_BUF_NONE;
		ccmdDataBufLruList.tailEntry = ccmdDataBufMapPtr->dataBuf[evictedEntry].prevEntry;

		ccmdDataBufMapPtr->dataBuf[evictedEntry].prevEntry = DATA_BUF_NONE;
		ccmdDataBufMapPtr->dataBuf[evictedEntry].nextEntry = ccmdDataBufLruList.headEntry;
		ccmdDataBufMapPtr->dataBuf[ccmdDataBufLruList.headEntry].prevEntry = evictedEntry;
		ccmdDataBufLruList.headEntry = evictedEntry;

	}
	else
	{
		ccmdDataBufMapPtr->dataBuf[evictedEntry].prevEntry = DATA_BUF_NONE;
		ccmdDataBufMapPtr->dataBuf[evictedEntry].nextEntry = DATA_BUF_NONE;
		ccmdDataBufLruList.headEntry = evictedEntry;
		ccmdDataBufLruList.tailEntry = evictedEntry;
	}

	SelectiveGetFromCcmdDataBufHashList(evictedEntry);

	return evictedEntry;
}
void UpdateCcmdDataBufEntryInfoBlockingReq(unsigned int bufEntry, unsigned int reqSlotTag)
{
	if(ccmdDataBufMapPtr->dataBuf[bufEntry].blockingReqTail != REQ_SLOT_TAG_NONE)
	{
		reqPoolPtr->reqPool[reqSlotTag].prevBlockingReq = ccmdDataBufMapPtr->dataBuf[bufEntry].blockingReqTail;
		reqPoolPtr->reqPool[reqPoolPtr->reqPool[reqSlotTag].prevBlockingReq].nextBlockingReq  = reqSlotTag;
	}

	ccmdDataBufMapPtr->dataBuf[bufEntry].blockingReqTail = reqSlotTag;

	GetTimeBlockedBufferDef(reqPoolPtr->reqPool[reqSlotTag].nvmeCmdSlotTag);
}

void PutToCcmdDataBufHashList(unsigned int bufEntry)
{
	unsigned int hashEntry;

	hashEntry = FindCcmdDataBufHashTableEntry(ccmdDataBufMapPtr->dataBuf[bufEntry].logicalSliceAddr);

	if(ccmdDataBufHashTablePtr->dataBufHash[hashEntry].tailEntry != DATA_BUF_NONE)
	{
		ccmdDataBufMapPtr->dataBuf[bufEntry].hashPrevEntry = ccmdDataBufHashTablePtr->dataBufHash[hashEntry].tailEntry ;
		ccmdDataBufMapPtr->dataBuf[bufEntry].hashNextEntry = REQ_SLOT_TAG_NONE;
		ccmdDataBufMapPtr->dataBuf[ccmdDataBufHashTablePtr->dataBufHash[hashEntry].tailEntry].hashNextEntry = bufEntry;
		ccmdDataBufHashTablePtr->dataBufHash[hashEntry].tailEntry = bufEntry;
	}
	else
	{
		ccmdDataBufMapPtr->dataBuf[bufEntry].hashPrevEntry = REQ_SLOT_TAG_NONE;
		ccmdDataBufMapPtr->dataBuf[bufEntry].hashNextEntry = REQ_SLOT_TAG_NONE;
		ccmdDataBufHashTablePtr->dataBufHash[hashEntry].headEntry = bufEntry;
		ccmdDataBufHashTablePtr->dataBufHash[hashEntry].tailEntry = bufEntry;
	}
}
void SelectiveGetFromCcmdDataBufHashList(unsigned int bufEntry)
{
	if(ccmdDataBufMapPtr->dataBuf[bufEntry].logicalSliceAddr != LSA_NONE)
	{
		unsigned int prevBufEntry, nextBufEntry, hashEntry;

		prevBufEntry =  ccmdDataBufMapPtr->dataBuf[bufEntry].hashPrevEntry;
		nextBufEntry =  ccmdDataBufMapPtr->dataBuf[bufEntry].hashNextEntry;
		hashEntry = FindCcmdDataBufHashTableEntry(ccmdDataBufMapPtr->dataBuf[bufEntry].logicalSliceAddr);

		if((nextBufEntry != DATA_BUF_NONE) && (prevBufEntry != DATA_BUF_NONE))
		{
			ccmdDataBufMapPtr->dataBuf[prevBufEntry].hashNextEntry = nextBufEntry;
			ccmdDataBufMapPtr->dataBuf[nextBufEntry].hashPrevEntry = prevBufEntry;
		}
		else if((nextBufEntry == DATA_BUF_NONE) && (prevBufEntry != DATA_BUF_NONE))
		{
			ccmdDataBufMapPtr->dataBuf[prevBufEntry].hashNextEntry = DATA_BUF_NONE;
			ccmdDataBufHashTablePtr->dataBufHash[hashEntry].tailEntry = prevBufEntry;
		}
		else if((nextBufEntry != DATA_BUF_NONE) && (prevBufEntry == DATA_BUF_NONE))
		{
			ccmdDataBufMapPtr->dataBuf[nextBufEntry].hashPrevEntry = DATA_BUF_NONE;
			ccmdDataBufHashTablePtr->dataBufHash[hashEntry].headEntry = nextBufEntry;
		}
		else
		{
			ccmdDataBufHashTablePtr->dataBufHash[hashEntry].headEntry = DATA_BUF_NONE;
			ccmdDataBufHashTablePtr->dataBufHash[hashEntry].tailEntry = DATA_BUF_NONE;
		}
	}
}

