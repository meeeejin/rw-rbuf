#include "xil_printf.h"
#include "memory_map.h"
#include "nvme/host_lld.h"
#include "request_schedule.h"

P_CUSTOM_IO_CMD_COMPLETE_TABLE customIoCmdCompleteTablePtr;

void InitCustomIoCmdCompleteTable()
{
	unsigned int cmdSlotTag;
	customIoCmdCompleteTablePtr = (P_CUSTOM_IO_CMD_COMPLETE_TABLE) CUSTOM_IO_CMD_COMPLETE_TABLE_ADDR;

	for(cmdSlotTag=0; cmdSlotTag < MAX_CMD_SLOT_TAG; cmdSlotTag++)
	{
		customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notIssuedRxCount = 0;
		customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedRxCount = 0;
		customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notIssuedTxCount =0;
		customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedTxCount = 0;
		customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedNANDCount = 0;
		customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].reserved0 = 0;
	}
}

void AllocateNotIssuedRxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int devAddr)
{
	customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notIssuedRxCount++;
#if (PRINT_DEBUG_MSG == 1)
	xil_printf("Allo Rx Custom Io (Issue) CmdSlotTag:%d Cnt:%d devAddr:%08x\r\n", cmdSlotTag,
			customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notIssuedRxCount, devAddr);
#endif
}

void AllocateNotIssuedTxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int devAddr)
{
	customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notIssuedTxCount++;
#if (PRINT_DEBUG_MSG == 1)
	xil_printf("Allo Tx Custom Io (Issue) CmdSlotTag:%d Cnt:%d devAddr:%08x\r\n", cmdSlotTag,
			customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notIssuedTxCount, devAddr);
#endif
}

void AllocateNotCompletedRxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int devAddr)
{
	customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedRxCount++;
#if (PRINT_DEBUG_MSG == 1)
	xil_printf("Allo Rx Custom Io CmdSlotTag:%d Cnt:%d devAddr:%08x\r\n", cmdSlotTag,
			customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedRxCount, devAddr);
#endif
}

void AllocateNotCompletedTxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int devAddr)
{
	customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedTxCount++;
#if (PRINT_DEBUG_MSG == 1)
	xil_printf("Allo Tx Custom Io (Complete) CmdSlotTag:%d Cnt:%d devAddr:%08x\r\n", cmdSlotTag,
			customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedTxCount, devAddr);
#endif
}

void AllocateNotCompletedNandReqforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int devAddr)
{
	customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedNANDCount++;
#if (PRINT_DEBUG_MSG == 1)
	char * nandReqCodeString;
	unsigned int reqCodeHex = reqPoolPtr->reqPool[reqSlotTag].reqCode;
	if(reqCodeHex == REQ_CODE_WRITE)
		nandReqCodeString = "WRITE";
	else if (reqCodeHex == REQ_CODE_READ)
		nandReqCodeString = "READ";
	else if (reqCodeHex == REQ_CODE_READ_TRANSFER)
		nandReqCodeString = "READ_Tx";
	else
		nandReqCodeString = "NONE";
	xil_printf("Allo NandReq %s Custom Io CmdSlotTag:%d Cnt:%d devAddr:%08x\r\n", nandReqCodeString, cmdSlotTag,
			customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedNANDCount, devAddr);
#endif
}

void IssuedRxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int reqCode)
{
	customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notIssuedRxCount--;
	reqPoolPtr->reqPool[reqSlotTag].nvmeDmaInfo.issuedFlag = REQ_INFO_ISSUED_FLAG_ON;
#if (PRINT_DEBUG_MSG == 1)
	xil_printf("Discount notIssuedRxCount cmdSlotTag:%d reqSlotTag:%d Cnt:%d\r\n", cmdSlotTag, reqSlotTag, customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notIssuedRxCount);
	PrintDataBuffer(reqSlotTag, 1);
#endif
}

void IssuedTxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int reqCode)
{
	customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notIssuedTxCount--;
	reqPoolPtr->reqPool[reqSlotTag].nvmeDmaInfo.issuedFlag = REQ_INFO_ISSUED_FLAG_ON;
#if (PRINT_DEBUG_MSG == 1)
	xil_printf("Discount notIssuedTxCount cmdSlotTag:%d reqSlotTag:%d Cnt:%d\r\n", cmdSlotTag, reqSlotTag, customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notIssuedTxCount);
	//xil_printf("Issue Tx Custom Io CmdSlotTag:%d reqSlotTag:%d\r\n", cmdSlotTag, reqSlotTag);
	PrintDataBuffer(reqSlotTag, 1);
#endif
}

void CompleteRxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int reqCode)
{
	customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedRxCount--;
#if (PRINT_DEBUG_MSG == 1)
	xil_printf("Discount notCompletedRxCount cmdSlotTag:%d reqSlotTag:%d Cnt:%d\r\n", cmdSlotTag, reqSlotTag, customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedRxCount);
	xil_printf("Complete Rx Custom Io CmdSlotTag:%d reqSlotTag:%d\r\n", cmdSlotTag, reqSlotTag);
	PrintDataBuffer(reqSlotTag, 1);
#endif
}

void CompleteTxforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int reqCode)
{
	NVME_COMPLETION nvmeCPL;
	nvmeCPL.statusFieldWord = 0;
	nvmeCPL.statusField.SCT = SCT_GENERIC_COMMAND_STATUS;
	nvmeCPL.statusField.SC = SC_SUCCESSFUL_COMPLETION;
	nvmeCPL.statusField.DNR = 1;
	customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedTxCount--;
#if (PRINT_DEBUG_MSG == 1)
	xil_printf("Discount notCompletedTxCount cmdSlotTag:%d reqSlotTag:%d Cnt:%d\r\n", cmdSlotTag, reqSlotTag, customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedTxCount);
#endif
	if(customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedTxCount == 0
			&& customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedRxCount == 0)
	{
		set_auto_nvme_cpl(cmdSlotTag, 0, nvmeCPL.statusFieldWord);
#if (PRINT_DEBUG_MSG == 1)
		xil_printf("Complete Tx Custom Io CmdSlotTag:%d reqSlotTag:%d NvmeBlockCnt:%d\r\n", cmdSlotTag, reqSlotTag,
				reqPoolPtr->reqPool[reqSlotTag].nvmeDmaInfo.numOfNvmeBlock);
		PrintDataBuffer(reqSlotTag, reqPoolPtr->reqPool[reqSlotTag].nvmeDmaInfo.numOfNvmeBlock);
#endif
	}
}

void CompleteNandReqforCustomIoCmd(unsigned int cmdSlotTag, unsigned int reqSlotTag, unsigned int reqCode)
{


	customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedNANDCount--;
#if (PRINT_DEBUG_MSG == 1)
	char * nandReqCodeString;
	unsigned int reqCodeHex = reqPoolPtr->reqPool[reqSlotTag].reqCode;
	if(reqCodeHex == REQ_CODE_WRITE)
		nandReqCodeString = "WRITE";
	else if (reqCodeHex == REQ_CODE_READ)
		nandReqCodeString = "READ";
	else if (reqCodeHex == REQ_CODE_READ_TRANSFER)
		nandReqCodeString = "READ_Tx";
	else
		nandReqCodeString = "NONE";
	xil_printf("Complete NandReq %s Custom Io CmdSlotTag:%d Cnt:%d devAddr:%08x reqCode:%x\r\n", nandReqCodeString, cmdSlotTag,
			customIoCmdCompleteTablePtr->customIoCmdCompleteEntry[cmdSlotTag].notCompletedNANDCount, GenerateDataBufAddr(reqSlotTag), reqCode);
#endif
}
