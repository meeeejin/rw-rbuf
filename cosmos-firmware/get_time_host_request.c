#include <assert.h>
#include "xil_printf.h"
#include "memory_map.h"
#include "nvme/host_lld.h"
#include "request_schedule.h"
#include <stdio.h>

P_HOST_REQUEST_TIME_TABLE hostRequestTimeTablePtr;
P_PRINT_BUFFER_TABLE printBuffer;
unsigned int bufferWritePoint;

unsigned int time_cnt = 0;
unsigned int total_r = 0;
unsigned int total_w = 0;

void InitHostReqTimeTable()
{
	unsigned int cmdSlotTag, buffEntry;
	hostRequestTimeTablePtr = (P_HOST_REQUEST_TIME_TABLE) HOST_REQUEST_TIME_TABLE_ADDR;
	printBuffer = (P_PRINT_BUFFER_TABLE) PRINT_BUFFER_TABLE_ADDR;

	for(cmdSlotTag=0; cmdSlotTag < MAX_CMD_SLOT_TAG; cmdSlotTag++)
	{
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].reqCode = ENTRY_NONE;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].fetch = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].blockedBufferDef = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqStart = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqEnd = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaStart = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaEnd = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaCnt = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqCnt = 0;
	}

	for(buffEntry = 0; buffEntry < PRINT_BUFFER_ENTRY_CNT; buffEntry++)
	{
		memset(printBuffer->buffer[buffEntry].d, 0, PRINT_BUFFER_DATA_SIZE);
	}
	bufferWritePoint = 0;
}
void AllocateHostReqTime(unsigned int cmdSlotTag, unsigned int reqCode)
{
	if(reqCode == REQ_CODE_WRITE)
		reqCode = REQ_CODE_RxDMA;
	else if(reqCode == REQ_CODE_READ)
		reqCode = REQ_CODE_TxDMA;
	else
		assert(!"[WARNING] Not supported reqCode. [WARNING]");

	if(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].reqCode == ENTRY_NONE)
	{
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].reqCode = reqCode;
		GetTimeFetch(cmdSlotTag);
	}
}
void AllocateNvmeDmaCnt(unsigned int cmdSlotTag)
{
	hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaCnt++;
}
void AllocateNandReqCntForBufferEviction(unsigned int cmdSlotTag)
{
	hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqCnt++;
}
void GetTimeFetch(unsigned int cmdSlotTag)
{
	if(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].fetch == 0)
	{
		XTime_GetTime(&hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].fetch);
	}
}
void GetTimeBlockedBufferDef(unsigned int cmdSlotTag)
{
	if(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].blockedBufferDef == 0)
		XTime_GetTime(&hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].blockedBufferDef);
}
void GetTimeBufferEvictNandReqStart(unsigned int cmdSlotTag, unsigned int reqSlotTag)
{
	if(reqPoolPtr->reqPool[reqSlotTag].reqCode == REQ_CODE_WRITE)
	{
		if(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqStart == 0)
			XTime_GetTime(&hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqStart);
	}
}
void GetTimeBufferEvictNandReqEnd(unsigned int cmdSlotTag, unsigned int reqSlotTag)
{
	if(reqPoolPtr->reqPool[reqSlotTag].reqCode == REQ_CODE_WRITE)
	{
		if(--hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqCnt == 0)
		{
			if(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqEnd == 0)
				XTime_GetTime(&hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqEnd);
		}
	}
}
void GetTimeHostReqNvmeDmaStart(unsigned int cmdSlotTag)
{
	if(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaStart == 0)
		XTime_GetTime(&hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaStart);
}
void GetTimeHostReqNvmeDmaEnd(unsigned int cmdSlotTag)
{
	if(--hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaCnt == 0)
	{
		if(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaEnd == 0)
		{
			XTime_GetTime(&hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaEnd);
			BufferItemValue(cmdSlotTag);
		}
	}
}
void ResetHostRequestTimeEntry(unsigned int cmdSlotTag)
{
	//if(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].reqCode != ENTRY_NONE)
	{
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].reqCode = ENTRY_NONE;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].fetch = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].blockedBufferDef = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqStart = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqEnd = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaStart = 0;
		hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaEnd = 0;
	}
	//else
		//assert(!"[WARNING] Not found cmdSlotTag [WARNING]");

}

void BufferItemValue(unsigned int cmdSlotTag)
{
	char command_r[8] = "read";
	char command_w[8] = "write";
	char command[8];
	unsigned int FetchToEvictionStart, ElapseEvictBuff, EvictToComplete;

	if(bufferWritePoint == PRINT_BUFFER_ENTRY_CNT)
	{
		//PrintTimeOfHostRequests();
		InitHostReqTimeTable();
		bufferWritePoint = 0;
	}

	if(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].reqCode == REQ_CODE_TxDMA)
	{
		//sprintf(command, "READ");
		strncpy(command, command_r, sizeof(command_r));
		total_r++;
	}
	else //hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].reqCode == REQ_CODE_RxDMA
	{
		//sprintf(command, "WRITE");
		strncpy(command, command_w, sizeof(command_w));
		total_w++;
	}

	FetchToEvictionStart = (unsigned int)(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].blockedBufferDef - hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].fetch);
	ElapseEvictBuff = (unsigned int)(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqEnd - hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nandReqStart);
	EvictToComplete = (unsigned int)(hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaEnd - hostRequestTimeTablePtr->hostRequestTimeEntry[cmdSlotTag].nvmeDmaStart);

	time_cnt++;
	if (time_cnt % 1000 == 0) {
		//sprintf(printBuffer->buffer[bufferWritePoint].d,
		xil_printf("HostReq %d Cmd %s FetchToEvictionStart %u us / %u ElapseEvictBuff %u us / %u EvictToComplete %u us / %u (r = %u, w = %u)\n",
				cmdSlotTag, command,
				FetchToEvictionStart / (COUNTS_PER_SECOND / 1000000), FetchToEvictionStart,
				ElapseEvictBuff / (COUNTS_PER_SECOND / 1000000), ElapseEvictBuff,
				EvictToComplete / (COUNTS_PER_SECOND / 1000000), EvictToComplete,
				total_r, total_w);
	}
	bufferWritePoint++;
	ResetHostRequestTimeEntry(cmdSlotTag);
}
void PrintTimeOfHostRequests()
{
	int i;
	for(i = 0; i < PRINT_BUFFER_ENTRY_CNT; i++)
	{
		xil_printf("%s\n", printBuffer->buffer[i].d);
	}
}
