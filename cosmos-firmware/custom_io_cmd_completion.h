#ifndef SRC_CUSTOM_IO_CMD_COMPLETION_H_
#define SRC_CUSTOM_IO_CMD_COMPLETION_H_

#define PRINT_DEBUG_MSG		0
#define MAX_CMD_SLOT_TAG	1024

typedef struct _CUSTOM_IO_CMD_COMPLETE_ENTRY
{
	unsigned int notIssuedRxCount: 16;
	unsigned int notCompletedRxCount: 16;
	unsigned int notIssuedTxCount: 16;
	unsigned int notCompletedTxCount: 16;
	unsigned int notCompletedNANDCount: 16;
	unsigned int reserved0: 16;
} CUSTOM_IO_CMD_COMPLETE_ENTRY, *P_CUSTOM_IO_CMD_COMPLETE_ENTRY;

typedef struct _CUSTOM_IO_CMD_COMPLETE_TABLE
{
	CUSTOM_IO_CMD_COMPLETE_ENTRY customIoCmdCompleteEntry[MAX_CMD_SLOT_TAG];
} CUSTOM_IO_CMD_COMPLETE_TABLE, *P_CUSTOM_IO_CMD_COMPLETE_TABLE;

void InitCustomIoCmdCompleteTable();
void AllocateNotIssuedRxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int devAddr);
void AllocateNotIssuedTxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int devAddr);
void AllocateNotCompletedRxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int devAddr);
void AllocateNotCompletedTxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int devAddr);
void AllocateNotCompletedNandReqforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int devAddr);
void IssuedRxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int reqCode);
void IssuedTxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int reqCode);
void CompleteRxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int reqCode);
void CompleteTxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int reqCode);
void CompleteNandReqforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int reqCode);

#endif /* SRC_CUSTOM_IO_CMD_COMPLETION_H_ */
