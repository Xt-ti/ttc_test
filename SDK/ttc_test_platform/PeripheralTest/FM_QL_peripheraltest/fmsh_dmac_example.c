/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/******************************************************************************
*
* @file  fmsh_dmac_example.c
*
* This file contains 
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   yl  12/20/2018  First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdlib.h>
#include <string.h>
#include "fmsh_ps_parameters.h"
#include "fmsh_dmac_example.h"
#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"
#include "cache.h"

/************************** Constant Definitions *****************************/

// Source memory location
#define SRC_MEMORY_BASE          0xE1FE0000///(0xE1FFA000)
// Destination memory location
#define DST_MEMORY_BASE          0xE1FF0000//(0xE1FFC000)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

// Declare global variables
FDmaPs_T g_DMA_dmac;
FDmaPs_Param_T g_DMA_param;
FDmaPs_Instance_T g_DMA_instance;

// define globle list
FMSH_LIST_HEAD(g_DMA_list);
u32 g_DMA_listNum;

static volatile BOOL s_DMA_tfrFlag;
static volatile BOOL s_DMA_errFlag;
static volatile BOOL s_DMA_srcTranFlag;
static volatile BOOL s_DMA_dstTranFlag;

// Function prototypes
void userListener(void *pDev, int eCode);
void userCallback(void *pDev, int eCode);
s32 FDmaPs_dmac_init(FDmaPs_T *pDmac);

//extern FPSGIC IntcInstance;

/************************** Function Prototypes ******************************/

void FDmaPs_IRQ (void *InstancePtr)
{
	FDmaPs_irqHandler((FDmaPs_T *)InstancePtr);
}

/*********************************************************************/
void userCallback(void *pDev, int eCode)
{
    // -------
    // This function is called by the IRQ handler.
    // Here we are going to set a global flag to inform the
    // application that the DMA transfer has completed.
    // -------
    s_DMA_tfrFlag = TRUE;
}

/*********************************************************************/
void userListener(void *pDev, int eCode)
{
    // Check the source of the interrupt
    if (eCode == Dmac_irq_err)
	{
        // Just inform the application that this interrupt has occured.
        s_DMA_errFlag = TRUE;
    }

    if (eCode == Dmac_irq_srctran)
	{
        // Just inform the application that this interrupt has occured.
        s_DMA_srcTranFlag = TRUE;
    }

    if (eCode == Dmac_irq_dsttran)
	{
        // Just inform the application that this interrupt has occured.
        s_DMA_dstTranFlag = TRUE;
    }

    if (eCode == Dmac_irq_block)
	{
        // We fully expect this interrupt to fire each time the DMA
        // controller has completed the transfer of a block of data.
        // For this example we are not concerned that a block has
        // completed, only that the transfer completes. So we do
        // nothing here.
    }
}

int FDmaPs_srcDstDataCompare(u32 sar, u32 dar, u32 block_ts, u32 block_num, enum FDmaPs_transferType transfer_type)
{
    s32 testStatus = FMSH_SUCCESS;
    int j, k;
    u32 dst_data, exp_data, *dst_addr, *src_addr;
	
    src_addr = (u32 *)sar;
    dst_addr = (u32 *)dar;
	if ((transfer_type == Dmac_transfer_row1) ||
		(transfer_type == Dmac_transfer_row5) ||
	    (transfer_type == Dmac_transfer_row6) ||
	    (transfer_type == Dmac_transfer_row8) ||
	    (transfer_type == Dmac_transfer_row10))
	{
        for (j = 0; j < block_ts; j++)
		{
            exp_data = *(u32 *) src_addr++;
            dst_data = *(volatile u32 *) dst_addr++;
            if (exp_data != dst_data)
			{
                testStatus = FMSH_FAILURE;
            	TRACE_OUT(DEBUG_OUT, "dst_addr = %x : dst_data = %x : exp_data = %x\r\n",
					      dst_addr-1, dst_data, exp_data);
				break;
            }
        }
	}
	else if ((transfer_type == Dmac_transfer_row2) ||
		     (transfer_type == Dmac_transfer_row9))
	{
		for (k = 0; k < block_num; k++)
		{
			// Row2 has auto-reload addressing on the destination
			if (k < block_num - 1)
			{
		        for (j = 0; j < block_ts; j++)
				{
		            (u32 *) src_addr++;
		        }
    		}
			else
			{
				dst_addr = (u32 *)dar;
		        for (j = 0; j < block_ts; j++)
				{
		            exp_data = *(u32 *) src_addr++;
		            dst_data = *(volatile u32 *) dst_addr++;
		            if (exp_data != dst_data)
					{
		                testStatus = FMSH_FAILURE;
		            	TRACE_OUT(DEBUG_OUT, "blkNum = %d, dst_addr = %x : dst_data = %x : exp_data = %x\r\n",
							      k, dst_addr-1, dst_data, exp_data);
						break;
		            }
		        }
			}
	    }
	}
	else if ((transfer_type == Dmac_transfer_row3) ||
		     (transfer_type == Dmac_transfer_row7))
	{
		for (k = 0; k < block_num; k++)
		{
			// Row3 has auto-reload addressing on the source
        	src_addr = (u32 *)sar;
	        for (j = 0; j < block_ts; j++)
			{
	            exp_data = *(u32 *) src_addr++;
	            dst_data = *(volatile u32 *) dst_addr++;
	            if (exp_data != dst_data)
				{
	                testStatus = FMSH_FAILURE;
	            	TRACE_OUT(DEBUG_OUT, "blkNum = %d, dst_addr = %x : dst_data = %x : exp_data = %x\r\n",
						      k, dst_addr-1, dst_data, exp_data);
					break;
	            }
	        }
	    }
	}
    else if (transfer_type == Dmac_transfer_row4)
	{
		// Row4 has auto-reload addressing on the source and destination
		// all the blocks has the same datas.
        for (j = 0; j < block_ts; j++)
		{
            exp_data = *(u32 *) src_addr++;
            dst_data = *(volatile u32 *) dst_addr++;
            if (exp_data != dst_data)
			{
                testStatus = FMSH_FAILURE;
            	TRACE_OUT(DEBUG_OUT, "blkNum = %d, dst_addr = %x : dst_data = %x : exp_data = %x\r\n",
					      k, dst_addr-1, dst_data, exp_data);
				break;
            }
        }
	}

	return testStatus;
}

void flush_dcache(u32 start, u32 stop)
{
    if (start & (64 - 1))
        start = ((start >> 6) - 1) << 6;

    if (stop & (64 - 1))
        stop = ((stop >> 6) + 1) << 6;
    
    flush_dcache_range((u64)start, (u64)stop);
}

int FDmaPs_setLliEntry(FDmaPs_T *pDmac, int num_blocks,
                             enum FDmaPs_transferType transType,
                             FMSH_listHead *listHead,
                             FDmaPs_ChannelConfig_T *pChConfig)
{
	u32 i;
	u32 sar, dar;
    enum FMSH_state ctl_llp_src_en;
    enum FMSH_state ctl_llp_dst_en;
	FDmaPs_LliItem_T *lliItem;
    u32 node_num;
	FMSH_listHead *pos, *p;
	
	// backup before LLI setting
	sar = pChConfig->sar;
	dar = pChConfig->dar;
	ctl_llp_src_en = pChConfig->ctl_llp_src_en;
	ctl_llp_dst_en = pChConfig->ctl_llp_dst_en;

    node_num = 0;
	for (i = 0; i < num_blocks; i++)
	{
		// create new node
		lliItem = (FDmaPs_LliItem_T *) malloc(sizeof(FDmaPs_LliItem_T));
		if(lliItem == NULL)
	    {
	        TRACE_OUT(DEBUG_OUT, "\nCann't create node!\r\n");
	        return FMSH_EINVAL;
	    }
		memset(lliItem, 0, sizeof(FDmaPs_LliItem_T));
		FMSH_INIT_LIST_HEAD(&lliItem->list);
        
#if PSOC_CACHE_ENABLE
    flush_dcache((u32)lliItem, (u32)lliItem + ((sizeof(FDmaPs_LliItem_T) * 4)));
#endif
		if (i == num_blocks - 1)
		{
			pChConfig->ctl_llp_dst_en = FMSH_clear;
			pChConfig->ctl_llp_src_en = FMSH_clear;
		}
		
		FDmaPs_addLliItem(listHead, lliItem, pChConfig);

		if (transType != Dmac_transfer_row7)
			pChConfig->sar = pChConfig->sar + pChConfig->ctl_block_ts * 4;

		if (transType != Dmac_transfer_row9)
			pChConfig->dar = pChConfig->dar + pChConfig->ctl_block_ts * 4;

        node_num++;
	}

	// recover after LLI setting
	pChConfig->ctl_llp_src_en = ctl_llp_src_en;
	pChConfig->ctl_llp_dst_en = ctl_llp_dst_en;
	pChConfig->sar = sar;
	pChConfig->dar = dar;
    
    // get the first LLI address by head node
    i = 0;
	pos = listHead;
	while (i < g_DMA_listNum)
	{
		p = pos->next;
		pos = p;
        
        i++;
	}
    lliItem = FMSH_LIST_ENTRY(pos, FDmaPs_LliItem_T, list);
    pChConfig->llp_loc = lliItem->llp >> 2;

    g_DMA_listNum += node_num;

	return 0;
}


void FDmaPs_delLliEntry(FMSH_listHead *head)
{
	FMSH_listHead *pos, *p;
	FDmaPs_LliItem_T *prev_entry;

	pos = head->next;
	while (pos != head)
	{
		p = pos->next;
		FMSH_listDelInit(pos);
		// free the memory
		prev_entry = FMSH_LIST_ENTRY(pos, FDmaPs_LliItem_T, list);
		free(prev_entry);
		pos = p;
	}

	FMSH_INIT_LIST_HEAD(head);
}

/*********************************************************************/
s32 dmac_transfer(FDmaPs_T *pDmac)
{
    int i, j, chIndex, errorCode;
    u32 blockSize, numBlocks, dataLen;
    u32 dst_data, exp_data, *dst_addr, *src_addr;
    enum FDmaPs_channelNumber chNum;
    enum FDmaPs_transferType transType;
    FDmaPs_ChannelConfig_T ch_config;  // Channel configuration struct.

    // Initialise the DMA controller
    FDmaPs_init(pDmac);

    // Transfer characteristics
    chNum = Dmac_channel0; // Select a DMA channel
    transType = Dmac_transfer_row1; // Select a transfer type
    blockSize = 480; // Select the size of a block
    numBlocks = 1; // Select the number of blocks
    dataLen = blockSize * numBlocks;

    // Initialise the memory regions on both the source and the
    // destination memories.
	src_addr = (u32 *)SRC_MEMORY_BASE;
	{
		for(i = 0; i < dataLen; i++)
	    {
	    	*src_addr = i + 1;
			src_addr++;
	    }
	}
	dst_addr = (u32 *)DST_MEMORY_BASE;
	{
		memset(dst_addr, 0, dataLen * sizeof(u32));
	}

    // Get the channel index from the enumerated type
    chIndex = FDmaPs_getChannelIndex(chNum);

    // START : Channel configuration
    // -------
    errorCode = FDmaPs_setTransferType(pDmac, chNum, transType);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: Failed to set the transfer type row %d\r\n", transType);
    }
    
    // Enable the interrupts on Channel x
    errorCode = FDmaPs_enableChannelIrq(pDmac, chNum);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: Failed to enable channel %d interrupts\r\n", chIndex);
    }

    // Initialise the channel configuration structure.
    errorCode = FDmaPs_getChannelConfig(pDmac, chNum, &ch_config);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: Failed to initialise configuration ");
        TRACE_OUT(DEBUG_OUT, "structure from the DMA registers on channel %d\r\n", chIndex);
    }

    // Change the configuration structure members to initialise the
    // DMA channel for the chosen transfer.
    // Set the Source and destination addresses
    ch_config.sar = SRC_MEMORY_BASE;
    ch_config.dar = DST_MEMORY_BASE;
    // Set the source and destination transfer width
    ch_config.ctl_src_tr_width = Dmac_trans_width_32;
    ch_config.ctl_dst_tr_width = Dmac_trans_width_32;
    // Set the Address increment type for the source and destination
    ch_config.ctl_sinc = Dmac_addr_increment;
    ch_config.ctl_dinc = Dmac_addr_increment;
    // Set the source and destination burst transaction length
    ch_config.ctl_src_msize = Dmac_msize_32;
    ch_config.ctl_dst_msize = Dmac_msize_32;
	// set scatter/gather enable and parameters
	ch_config.ctl_dst_scatter_en = FMSH_clear;
	ch_config.ctl_src_gather_en = FMSH_clear;
    // Set the block size for the DMA transfer
    // Block size is the number of words of size Dmac_trans_width
    ch_config.ctl_block_ts = blockSize;
    // Set the transfer device type and flow controller
    ch_config.ctl_tt_fc = Dmac_mem2mem_dma;

    // Write the new configuration setting into the DMA Controller device.
    errorCode = FDmaPs_setChannelConfig(pDmac, chNum, &ch_config);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: Failed to write configuration structure ");
        TRACE_OUT(DEBUG_OUT, "into the DMA controller registers on channel %d\r\n", chIndex);
    }
    // -------
    // END : Channel configuration

    // display the transfer information.
    TRACE_OUT(DEBUG_OUT, "/ -----\r\n");
    TRACE_OUT(DEBUG_OUT, "  Channel number       %d\r\n", chIndex);
    TRACE_OUT(DEBUG_OUT, "  Transfer type        Row %d\r\n", transType);
    TRACE_OUT(DEBUG_OUT, "  Block Size           %u\r\n", blockSize);
    TRACE_OUT(DEBUG_OUT, "  Number of blocks     %u\r\n", numBlocks);
    TRACE_OUT(DEBUG_OUT, "  Source address       %x\r\n", ch_config.sar);
    TRACE_OUT(DEBUG_OUT, "  Destination address  %x\r\n", ch_config.dar);
    TRACE_OUT(DEBUG_OUT, "/ -----\r\n");

    // Before beginning an interrupt driven transfer the user must
    // register a listener function in the DMA driver. 
    // -------
    FDmaPs_setListener(pDmac, chNum, userListener);
    
    // Initialise global flags to FALSE
    s_DMA_tfrFlag = FALSE;
    s_DMA_errFlag = FALSE;
    s_DMA_srcTranFlag = FALSE;
    s_DMA_dstTranFlag = FALSE;

#if PSOC_CACHE_ENABLE
    flush_dcache_range(SRC_MEMORY_BASE, (SRC_MEMORY_BASE + dataLen * 4));
    invalidate_dcache_range(DST_MEMORY_BASE, (DST_MEMORY_BASE + dataLen * 4));
#endif
    
    // Enable the DMA controller and begin the interrupt driven DMA transfer.
    FDmaPs_enable(pDmac);
    errorCode = FDmaPs_startTransfer(pDmac, chNum, numBlocks, userCallback);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: Failed to begin the interrupt transfer\r\n");
    }

    // Now we monitor the transfers progress and wait for completion
    // -------
    while (s_DMA_tfrFlag == FALSE)	// transfer is in progress
	{
        TRACE_OUT(DEBUG_OUT, ".");

        // Just checking here that no error condition interrupts
        // fire during the data transfer.

        if(s_DMA_errFlag == TRUE)
		{
            // Wasn't expecting this !!
            TRACE_OUT(DEBUG_OUT, "\nERROR: Recieved an err interrupt\r\n");
        }

        if (s_DMA_srcTranFlag == TRUE)
		{
            // This interrupt should not be unmasked when
            // a memory device is on the source !!
            TRACE_OUT(DEBUG_OUT, "\nERROR: Received an srcTran interrupt\r\n");
        }

        if (s_DMA_dstTranFlag == TRUE)
		{
            // This interrupt should not be unmasked when
            // a memory device is on the destination !!
            TRACE_OUT(DEBUG_OUT, "\nERROR: Received an dstTran interrupt\r\n");
        }
    }
	TRACE_OUT(DEBUG_OUT, "\r\n");

    // -------
    // Disable the DMA after transfer is complete.
    // -------
    FDmaPs_disable(pDmac);
	
	errorCode = FDmaPs_srcDstDataCompare(ch_config.sar, ch_config.dar, blockSize, numBlocks, transType);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: compare the src and dst data is Failed!\r\n");
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}

s32 dmac_transfer_LLI(FDmaPs_T *pDmac)
{
    int i, j, chIndex, errorCode;
    u32 blockSize, numBlocks, dataLen;
    u32 dst_data, exp_data, *dst_addr, *src_addr;
    enum FDmaPs_channelNumber chNum;
    enum FDmaPs_transferType transType;
    FDmaPs_ChannelConfig_T ch_config;  // Channel configuration struct.
	//FMSH_listHead list_head;

    // Initialise the DMA controller
    FDmaPs_init(pDmac);

    // Transfer characteristics
    chNum = Dmac_channel0; // Select a DMA channel
    transType = Dmac_transfer_row6; // Select a transfer type
    blockSize = 128; // Select the size of a block
    numBlocks = 3; // Select the number of blocks
    dataLen = blockSize * numBlocks;

    // Initialise the memory regions on both the source and the
    // destination memories.
	src_addr = (u32 *)SRC_MEMORY_BASE;
	{
		for(i = 0; i < dataLen; i++)
	    {
	    	*src_addr = i + 1;
			src_addr++;
	    }
	}
	dst_addr = (u32 *)DST_MEMORY_BASE;
	{
		memset(dst_addr, 0, dataLen * sizeof(u32));
	}

    // Get the channel index from the enumerated type
    chIndex = FDmaPs_getChannelIndex(chNum);

    // START : Channel configuration
    // -------
    errorCode = FDmaPs_setTransferType(pDmac, chNum, transType);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: Failed to set the transfer type row %d\r\n", transType);
    }
    
    // Enable the interrupts on Channel x
    errorCode = FDmaPs_enableChannelIrq(pDmac, chNum);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: Failed to enable channel %d interrupts\r\n", chIndex);
    }

    // Initialise the channel configuration structure.
    errorCode = FDmaPs_getChannelConfig(pDmac, chNum, &ch_config);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: Failed to initialise configuration ");
        TRACE_OUT(DEBUG_OUT, "structure from the DMA registers on channel %d\r\n", chIndex);
    }

    // Change the configuration structure members to initialise the
    // DMA channel for the chosen transfer.
    // Set the Source and destination addresses
    ch_config.sar = SRC_MEMORY_BASE;
    ch_config.dar = DST_MEMORY_BASE;
    // Set the source and destination transfer width
    ch_config.ctl_src_tr_width = Dmac_trans_width_32;
    ch_config.ctl_dst_tr_width = Dmac_trans_width_32;
    // Set the Address increment type for the source and destination
    ch_config.ctl_sinc = Dmac_addr_increment;
    ch_config.ctl_dinc = Dmac_addr_increment;
    // Set the source and destination burst transaction length
    ch_config.ctl_src_msize = Dmac_msize_32;
    ch_config.ctl_dst_msize = Dmac_msize_32;
	// set scatter/gather enable and parameters
	ch_config.ctl_dst_scatter_en = FMSH_clear;
	ch_config.ctl_src_gather_en = FMSH_clear;
    // Set the block size for the DMA transfer
    // Block size is the number of words of size Dmac_trans_width
    ch_config.ctl_block_ts = blockSize;
    // Set the transfer device type and flow controller
    ch_config.ctl_tt_fc = Dmac_mem2mem_dma;

	// set LLI items for multi blocks
	if ((numBlocks > 1) && (transType >= Dmac_transfer_row6))
	{
	    g_DMA_listNum = 0;
		errorCode = FDmaPs_setLliEntry(pDmac, numBlocks, transType, &g_DMA_list, &ch_config);
		if (errorCode != 0)
		{
	        TRACE_OUT(DEBUG_OUT, "ERROR: Failed to set LLI item\r\n");
	    }
	}

    // Write the new configuration setting into the DMA Controller device.
    errorCode = FDmaPs_setChannelConfig(pDmac, chNum, &ch_config);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: Failed to write configuration structure ");
        TRACE_OUT(DEBUG_OUT, "into the DMA controller registers on channel %d\r\n", chIndex);
    }
    // -------
    // END : Channel configuration

    // display the transfer information.
    TRACE_OUT(DEBUG_OUT, "/ -----\r\n");
    TRACE_OUT(DEBUG_OUT, "  Channel number       %d\r\n", chIndex);
    TRACE_OUT(DEBUG_OUT, "  Transfer type        Row %d\r\n", transType);
    TRACE_OUT(DEBUG_OUT, "  Block Size           %u\r\n", blockSize);
    TRACE_OUT(DEBUG_OUT, "  Number of blocks     %u\r\n", numBlocks);
    TRACE_OUT(DEBUG_OUT, "  Source address       %x\r\n", ch_config.sar);
    TRACE_OUT(DEBUG_OUT, "  Destination address  %x\r\n", ch_config.dar);
    TRACE_OUT(DEBUG_OUT, "/ -----\r\n");

    // Before beginning an interrupt driven transfer the user must
    // register a listener function in the DMA driver. 
    // -------
    FDmaPs_setListener(pDmac, chNum, userListener);
    
    // Initialise global flags to FALSE
    s_DMA_tfrFlag = FALSE;
    s_DMA_errFlag = FALSE;
    s_DMA_srcTranFlag = FALSE;
    s_DMA_dstTranFlag = FALSE;

#if PSOC_CACHE_ENABLE
    flush_dcache_range(SRC_MEMORY_BASE, (SRC_MEMORY_BASE + dataLen * 4));
    invalidate_dcache_range(DST_MEMORY_BASE, (DST_MEMORY_BASE + dataLen * 4));
#endif
    
    // Enable the DMA controller and begin the interrupt driven DMA transfer.
    FDmaPs_enable(pDmac);
    errorCode = FDmaPs_startTransfer(pDmac, chNum, numBlocks, userCallback);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: Failed to begin the interrupt transfer\r\n");
    }

    // Now we monitor the transfers progress and wait for completion
    // -------
    while (s_DMA_tfrFlag == FALSE)	// transfer is in progress
	{
        TRACE_OUT(DEBUG_OUT, ".");

        // Just checking here that no error condition interrupts
        // fire during the data transfer.

        if(s_DMA_errFlag == TRUE)
		{
            // Wasn't expecting this !!
            TRACE_OUT(DEBUG_OUT, "\nERROR: Recieved an err interrupt\r\n");
        }

        if (s_DMA_srcTranFlag == TRUE)
		{
            // This interrupt should not be unmasked when
            // a memory device is on the source !!
            TRACE_OUT(DEBUG_OUT, "\nERROR: Received an srcTran interrupt\r\n");
        }

        if (s_DMA_dstTranFlag == TRUE)
		{
            // This interrupt should not be unmasked when
            // a memory device is on the destination !!
            TRACE_OUT(DEBUG_OUT, "\nERROR: Received an dstTran interrupt\r\n");
        }
    }
	TRACE_OUT(DEBUG_OUT, "\r\n");

    // -------
    // Disable the DMA after transfer is complete.
    // -------
    FDmaPs_disable(pDmac);

	// delete all LLI
	if (transType >= Dmac_transfer_row6)
	{
		FDmaPs_delLliEntry(&g_DMA_list);
	}
	
	errorCode = FDmaPs_srcDstDataCompare(ch_config.sar, ch_config.dar, blockSize, numBlocks, transType);
    if (errorCode != 0)
	{
        TRACE_OUT(DEBUG_OUT, "ERROR: compare the src and dst data is Failed!\r\n");
        return FMSH_FAILURE;
    }

    return FMSH_SUCCESS;
}


s32 FDmaPs_example(void)
{
    s32 status = FMSH_FAILURE;
    FDmaPs_Config *pDmaCfg;
    FDmaPs_T *pDmac = &g_DMA_dmac;
    FDmaPs_Instance_T *pInstance = &g_DMA_instance;
    FDmaPs_Param_T *pParam = &g_DMA_param;

    memset(pInstance, 0, sizeof(FDmaPs_Instance_T));
    memset(pParam, 0, sizeof(FDmaPs_Param_T));

    /* Initialize the DMA Driver */
    pDmaCfg = FDmaPs_LookupConfig(FPAR_DMAPS_DEVICE_ID);
    if (pDmaCfg == NULL) {
        return FMSH_FAILURE;
    }
    FDmaPs_initDev(pDmac, pInstance, pParam, pDmaCfg);
    status = FGicPs_registerInt(&IntcInstance, DMA_INT_ID,
              (FMSH_InterruptHandler)FDmaPs_IRQ, pDmac);
    if (status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
	status = FDmaPs_autoCompParams(pDmac);
    if (status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    
	TRACE_OUT(DEBUG_OUT, "Begin to run DMA test example.\r\n");
	TRACE_OUT(DEBUG_OUT, "/ -------\r\n");
    
	status = dmac_transfer(pDmac);
    if (status != FMSH_SUCCESS)
        TRACE_OUT(DEBUG_OUT, "Run DMA single block test failed.\r\n");
	status = dmac_transfer_LLI(pDmac);
    if (status != FMSH_SUCCESS)
        TRACE_OUT(DEBUG_OUT, "Run DMA multi block(LLI mode) test failed.\r\n");

    if (status == FMSH_SUCCESS) {
        TRACE_OUT(DEBUG_OUT, "Run DMA test example success.\r\n");
        TRACE_OUT(DEBUG_OUT, "/ -------\r\n");
    }
	return status;
}

