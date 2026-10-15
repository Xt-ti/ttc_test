/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/******************************************************************************
*
* @file  fmsh_sdmmc_example.c
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fmsh_ps_parameters.h"
#include "fmsh_sdmmc_example.h"
#include "fmsh_dmac_example.h"
#include "fmsh_dmac.h"
#include "fmsh_gic.h"
#include "fmsh_gic_hw.h"

/************************** Constant Definitions *****************************/

/**  Number of errors displayed */
#define NB_ERRORS            5
/** Split R/W to 2, first R/W 4 blocks then remaining */
#define NB_SPLIT_MULTI       4
/** Maximum number of blocks read once */
#define MAX_MULTI_BLOCKS     128

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

FSdmmcPs_T g_SDMMC;
extern FDmaPs_T g_DMA_dmac;

static u32 nbErrors;

/************************** Function Prototypes ******************************/

static u8 MaxErrorBreak(u8 halt)
{
    if (NB_ERRORS)
    {
        if (nbErrors ++ > NB_ERRORS)
        {

            while(halt);

            nbErrors = 0;
            return 1;
        }
    }
    return 0;
}

int FSdmmcPs_irqHandler(FSdmmcPs_T *pSdmmc)
{
	u32 retval;
    u32 irq_active;
    u32 mask = 0;
	FSdmmcPs_SdCmd_T *pCmd;
	FSdmmcPs_Instance_T *pInstance;
	pInstance = (FSdmmcPs_Instance_T *)pSdmmc->instance;
	pCmd = &pInstance->sdCmd;

    // read the interrupt status register
	irq_active = FSdmmcPs_getMaskIntStat(pSdmmc);

	retval = FALSE;
	
    // card detect interrupt
    if (irq_active & RAW_INT_STATUS_CD)
	{
		// clear CD int;
		FSdmmcPs_setRawIntStat(pSdmmc, MASK_INT_STATUS_CD);

		// disable CD int
		//mask = FSdmmcPs_getIntMask(pSdmmc);
		//FSdmmcPs_setIntMask(pSdmmc, mask & (~MASK_INT_STATUS_CD));

		return 0;
	}
	/* Check errors */
    if (irq_active & INT_STATUS_ERRORS)
    {
        if (irq_active & RAW_INT_STATUS_RTO)
        {
            pCmd->ret_status = SDMMC_ERROR_NORESPONSE;
			// clear RTO int;
			FSdmmcPs_setRawIntStat(pSdmmc, RAW_INT_STATUS_RTO);

			// disable RTO int
			mask = FSdmmcPs_getIntMask(pSdmmc);
			FSdmmcPs_setIntMask(pSdmmc, mask & (~MASK_INT_STATUS_RTO));
        }
    }
	// clear err int;
	FSdmmcPs_setRawIntStat(pSdmmc, (irq_active & INT_STATUS_ERRORS));

    /* Check command complete */
    if (irq_active & RAW_INT_STATUS_CMDD)
    {
		// clear CMDO int;
		//FSdmmcPs_setRawIntStat(pSdmmc, RAW_INT_STATUS_CMDD);
		
		// disable CMDD int
		mask = FSdmmcPs_getIntMask(pSdmmc);
		FSdmmcPs_setIntMask(pSdmmc, mask & (~MASK_INT_STATUS_CMDD));
    }
   
    /* Check if TX request */
    if (irq_active & RAW_INT_STATUS_TXDR)
    {
		// clear TXDR int;
		FSdmmcPs_setRawIntStat(pSdmmc, RAW_INT_STATUS_TXDR);

		// disable TXDR int
		mask = FSdmmcPs_getIntMask(pSdmmc);
		FSdmmcPs_setIntMask(pSdmmc, mask & (~MASK_INT_STATUS_TXDR));
    }
    /* Check if RX request */
    if (irq_active & RAW_INT_STATUS_RXDR)
    {
		// clear RXDR int;
		FSdmmcPs_setRawIntStat(pSdmmc, RAW_INT_STATUS_RXDR);

		// disable RXDR int
		mask = FSdmmcPs_getIntMask(pSdmmc);
		FSdmmcPs_setIntMask(pSdmmc, mask & (~MASK_INT_STATUS_RXDR));
    }
     /* Check if Auto cmd done */
    if (irq_active & RAW_INT_STATUS_ACD)
    {
		// clear ACD int;
		FSdmmcPs_setRawIntStat(pSdmmc, RAW_INT_STATUS_ACD);
    }
    
    /* Check if transfer finished */
    if (irq_active & RAW_INT_STATUS_DTO)
    {
		// clear DTO int;
		//FSdmmcPs_setRawIntStat(pSdmmc, RAW_INT_STATUS_DTO);

		// disable DTO int
		mask = FSdmmcPs_getIntMask(pSdmmc);
		FSdmmcPs_setIntMask(pSdmmc, mask & (~MASK_INT_STATUS_DTO));
    }

	retval = TRUE;
    return retval;
}

void FSdmmcPs_IRQ (void *InstancePtr)
{
	FSdmmcPs_irqHandler((FSdmmcPs_T *)InstancePtr);
}

void FSdmmcPs_initDmac(FSdmmcPs_T *pSdmmc)
{
	FSdmmcPs_Instance_T *pInstance;
	pInstance = (FSdmmcPs_Instance_T *)pSdmmc->instance;

	// Initialize the DMA controller
	FDmaPs_init(pInstance->pDma);
	FDmaPs_enable(pInstance->pDma);
}

u32 FSdmmcPs_initSdmmcCtrl(FSdmmcPs_T *pSdmmc, enum SDMMC_TransMode trans_mode)
{
	int ret = FMSH_SUCCESS;
    /* Reset the chip. */
    ret = FSdmmcPs_sdControllerReset(pSdmmc);
	if (ret < 0)
		return FMSH_FAILURE;
    ret = FSdmmcPs_sdFifoReset(pSdmmc);
	if (ret < 0)
		return FMSH_FAILURE;
	
	/* set fifo */
	FSdmmcPs_setDmaMsize(pSdmmc, 2);
	FSdmmcPs_setRXwmark(pSdmmc, 7);
	FSdmmcPs_setTXwmark(pSdmmc, 8);
	/* clear int */
	FSdmmcPs_setRawIntStat(pSdmmc, 0xFFFFFFFF);
	/* enable int */
	FSdmmcPs_enableInt(pSdmmc);
    /* mask interrupt disable */
	FSdmmcPs_disableIntMask(pSdmmc);

	if (trans_mode == sdmmc_trans_mode_dw_dma)
	{
		FSdmmcPs_enableDMA(pSdmmc);
	}

	return ret;
}

s32 FSdmmcPs_cardInitializeVerify(FSdmmcPs_T *pSdmmc, FDmaPs_T *dev_dmac, enum SDMMC_TransMode trans_mode)
{
	s32 Status = FMSH_FAILURE;
	
	FSdmmcPs_instanceReset(pSdmmc, dev_dmac);
	Status = FSdmmcPs_initSdmmcCtrl(pSdmmc, trans_mode);
	if (Status != FMSH_SUCCESS)
	{
		return Status;
	}
	if (trans_mode == sdmmc_trans_mode_dw_dma)
		FSdmmcPs_initDmac(pSdmmc);
	Status = FSdmmcPs_initCard(pSdmmc);

	return Status;
}

s32 FSdmmcPs_cardDataTransfer(FSdmmcPs_T *pSdmmc, enum SDMMC_TransMode trans_mode, u8 wr, u8 rd)
{
    u8 error = 0;
    u32 i, test_cnt = 0, errcnt = 0;
    u32 multiBlock, block, splitMulti, totalBlockNum;
	u32 dataLen, rwDataLen;
	char8 *pWriteBuf, *pReadBuf;
	
    /* Perform tests on each block */
    multiBlock = 0;
	totalBlockNum = 100;

	pWriteBuf = (char8 *)0xE1Fe8000;
	pReadBuf = (char8 *)0xE1FF8000;
	
    for (block = 0; block < totalBlockNum; block += multiBlock)
	{
        /* Perform different single or multiple bloc operations */
		if (test_cnt < 3)
		{
			multiBlock = 1;
		}
		else
		{
	        if (multiBlock >= 8)
	    	{
				multiBlock = MAX_MULTI_BLOCKS;
	    	}
	        else
	    	{
	    		multiBlock ++;
	    	}
		}

        /* Multi-block adjustment */
        if (block + multiBlock > totalBlockNum)
		{
            multiBlock = totalBlockNum - block;
        }

        /* ** Perform single block or multi block transfer */
        TRACE_OUT(DEBUG_OUT, "-I- Testing block [%6u - %6u] ...\r\n", block, (block + multiBlock -1));
		
		dataLen = SDMMC_BLOCK_SIZE * multiBlock;

        if (wr) 
		{
			memset(pWriteBuf, 0, dataLen * sizeof(char8));
	        /* - Write a checkerboard pattern on the block */
	        for (i = 0; i < dataLen; i++)
			{
	            if ((i & 1) == 0) 
					pWriteBuf[i] = (i & 0x55);
	            else
	            	pWriteBuf[i] = (i & 0xAA);
	        }
			
            for (i = 0; i < multiBlock; )
			{
                splitMulti = ((multiBlock - i) > NB_SPLIT_MULTI) ? NB_SPLIT_MULTI : (multiBlock - i);
				rwDataLen = SDMMC_BLOCK_SIZE * splitMulti;
				
                error = FSdmmcPs_writeFun(pSdmmc, block + i, &pWriteBuf[i * SDMMC_BLOCK_SIZE], rwDataLen, trans_mode);
                if (error)
					break;
				else
                	i += splitMulti;
            }
            if (error)
            {
                TRACE_OUT(DEBUG_OUT, "-E- 2. Write block #%u(%u+%u): %d\n\r",  (block+i), block, i, error);
                if(MaxErrorBreak(0))
					return FMSH_FAILURE;
				else
	                continue; /* Skip following test */
            }
        }

        if (rd)
		{
            /* - Read back the data to check the write operation */
            memset(pReadBuf, 0, dataLen * sizeof(char8));
            for (i = 0; i < multiBlock; )
			{
                splitMulti = ((multiBlock - i) > NB_SPLIT_MULTI) ? NB_SPLIT_MULTI : (multiBlock - i);
				rwDataLen = SDMMC_BLOCK_SIZE * splitMulti;
				
                error = FSdmmcPs_readFun(pSdmmc, block + i, &pReadBuf[i * SDMMC_BLOCK_SIZE], rwDataLen, trans_mode);
                if (error) 
					break;
				else
                	i += splitMulti;
            }
            if (error)
            {
                TRACE_OUT(DEBUG_OUT, "-E- 2. Read block #%u(%u+%u): %d\n\r", (block + i), block, i, error);
                if(MaxErrorBreak(0))
					return FMSH_FAILURE;
				else
	                continue; /* Skip following test */
            }
            errcnt = 0;
            for (i=0; i < dataLen; i++)
			{
                if (pReadBuf[i] != pWriteBuf[i])
                {
                    uint32_t j, js;
                    TRACE_OUT(DEBUG_OUT, "\n\r-E- 2.%d. Data @ %u (0x%x)\n\r", errcnt, i, i);
                    TRACE_OUT(DEBUG_OUT, "  -Src:");
                    js = (i > 8) ? (i - 8) : 0;
                    for (j = js; j < i + 8; j ++)
                        TRACE_OUT(DEBUG_OUT, " %02x", pWriteBuf[j]);
                    TRACE_OUT(DEBUG_OUT, "\n\r  -Dat:");
                    for (j = js; j < i + 8; j ++)
                        TRACE_OUT(DEBUG_OUT, "%c%02x", (i == j) ? '!' : ' ', pReadBuf[j]);
                    TRACE_OUT(DEBUG_OUT, "\n\r");
                    if(MaxErrorBreak(0))
						return FMSH_FAILURE;
                    // Only find first 3 verify error.
                    if (errcnt ++ >= 3)
                        break;
                }
            }
        }

		test_cnt++;
    }

    return FMSH_SUCCESS;
}

s32 FSdmmcPs_example(void)
{
	s32 status = FMSH_FAILURE;
    u32 int_id;
    enum SDMMC_TransMode trans_mode;
    FSdmmcPs_Config *pSdCfg;
	FSdmmcPs_Instance_T instance;
    FDmaPs_Config *pDmaCfg;
    FDmaPs_T *pDmac = &g_DMA_dmac;
	FDmaPs_Param_T dma_param;
	FDmaPs_Instance_T dma_instance;

    int_id = SDMCC0_INT_ID;
    trans_mode = sdmmc_trans_mode_dw_dma;
    
    /* Initialize the SD Driver */
    pSdCfg = FSdmmcPs_LookupConfig(FPAR_SDPS_0_DEVICE_ID);
    if (pSdCfg == NULL)
    {
		return FMSH_FAILURE;
    }
	FSdmmcPs_initDev(&g_SDMMC, &instance, pSdCfg);
	status = FGicPs_registerInt(&IntcInstance, int_id,
	          (FMSH_InterruptHandler)FSdmmcPs_IRQ, &g_SDMMC);
	if (status != FMSH_SUCCESS)
	{
        return FMSH_FAILURE;
	}
	
	TRACE_OUT(DEBUG_OUT, "Begin to run SDMMC test example.\r\n");
	TRACE_OUT(DEBUG_OUT, "/ -------\r\n");

    /* Initialize the DMA Driver */
    pDmaCfg = FDmaPs_LookupConfig(FPAR_DMAPS_DEVICE_ID);
    if (pDmaCfg == NULL) {
        return FMSH_FAILURE;
    }
    FDmaPs_initDev(pDmac, &dma_instance, &dma_param, pDmaCfg);
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
	
	status = FSdmmcPs_cardInitializeVerify(&g_SDMMC, pDmac, trans_mode);
	if (status != FMSH_SUCCESS)
	{
		TRACE_OUT(DEBUG_OUT, "SDMMC test example FAILED.\r\n");
        return FMSH_FAILURE;
	}
    
	status = FSdmmcPs_cardDataTransfer(&g_SDMMC, trans_mode, 1, 1);
		
	return status;
}

