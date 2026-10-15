/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/******************************************************************************
*
* @file  fmsh_smc_example.c
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
#include "fmsh_common.h"
#include "fmsh_ps_parameters.h"
#include "fmsh_smc_example.h"

/************************** Constant Definitions *****************************/

#define NOR_DATA_BUS_WIDTH  (8) // 16
#define FLASH_TEST_SIZE	    (4048)
#define SMC_SRAM            (0)
#define SMC_NORFLASH        (1)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

FSmcPs_T g_SMC;
static u8 ReadBuffer[FLASH_TEST_SIZE]; /* Buffer used to store the data read. */
static u8 WriteBuffer[FLASH_TEST_SIZE]; /* Write buffer. */

/************************** Function Prototypes ******************************/

void FSmcPs_initCtrl(FSmcPs_T *pSmc, u8 dev_sel)
{
    u32 reg_tmp;
	u32 memctl_baseAddr;
	memctl_baseAddr = (u32)pSmc->base_address;

    reg_tmp = 0x12fd4;//0x12354;
    FMSH_WriteReg(memctl_baseAddr, SMC_SMTMGR_SET0, reg_tmp);
    FMSH_WriteReg(memctl_baseAddr, SMC_SMTMGR_SET1, reg_tmp);
    FMSH_WriteReg(memctl_baseAddr, SMC_SMTMGR_SET2, reg_tmp);

    // 4A: flash, 256Mb; 2A: sram, 256Mb;
    if (dev_sel == SMC_NORFLASH)
    	reg_tmp = 0x4A;
	else if (dev_sel == SMC_SRAM)
		reg_tmp = 0x2A;
	
    FMSH_WriteReg(memctl_baseAddr, SMC_SMSKRx(0), reg_tmp);
    FMSH_WriteReg(memctl_baseAddr, SMC_SMSKRx(1), reg_tmp);

    reg_tmp = 0x1201;
    FMSH_WriteReg(memctl_baseAddr, SMC_SMCTLR, reg_tmp);	 
}


int FSmcPs_init(FSmcPs_T *pSmc, u8 chip_sel, u8 bus_width, u8 dev_sel)
{
	int rst_ins = 0;
	// Initialize the Memory controller
    FSmcPs_initCtrl(pSmc, dev_sel);

	// Reset the flash instance structure
    rst_ins = FSmcPs_resetInstance(pSmc, chip_sel, bus_width);
	return rst_ins;
}

s32 FSmcPs_progVerify(FSmcPs_T *pSmc, u32 start_sector, u32 verify_sec_num)
{
    u32 i, startAddr, secNum, secSize;
	u32 startSec, endSec, secNumMax;
	u32 data_cnt;
	int ret = 0, data_comp;
	FSmcPs_Instance_T *pInstance = (FSmcPs_Instance_T *)pSmc->instance;

    // Display test information
    TRACE_OUT(DEBUG_OUT, "Begin to test SMC0 program function.\r\n");
    TRACE_OUT(DEBUG_OUT, "/ -------\r\n");

	// Initialize the device
	ret = FSmcPs_init(pSmc, 0, NOR_DATA_BUS_WIDTH, SMC_NORFLASH);
	if (ret == FMSH_ENXIO)
	{
		TRACE_OUT(DEBUG_OUT, "ERROR: Failed to reset instance.\r\n");
		return FMSH_FAILURE;
	}
	
	/* Get CFI data */
	ret = NorFlash_CFI_Detect(&pInstance->norFlash, NOR_DATA_BUS_WIDTH);
	if (ret != 0)
	{
		TRACE_OUT(DEBUG_OUT, "ERROR: Failed to detect CFI.\r\n");
		return FMSH_FAILURE;
	}
	secNumMax = NorFlash_GetDeviceNumOfBlocks(&pInstance->norFlash);
	secSize = NorFlash_GetDeviceMinBlockSize(&pInstance->norFlash);
    TRACE_OUT(DEBUG_OUT, "NorFlash0's sector number is %d, sector size is 0x%08x.\r\n", secNumMax, secSize);

	/*
	 * Reset the Flash Device. This clears the Status registers and puts
	 * the device in Read mode.
	 */
	NORFLASH_Reset(&pInstance->norFlash, 0);

	/* read device ID */
	NORFLASH_ReadDeviceID(&pInstance->norFlash);
    TRACE_OUT(DEBUG_OUT, "NorFlash0 ID: 0x%04x, 0x%04x\r\n", 
			  pInstance->norFlash.manufacture_id, pInstance->norFlash.device_id1);
	
	startSec = start_sector;
	endSec = startSec + verify_sec_num;
	if (endSec > secNumMax)
		endSec = secNumMax;

    /*
     * Prepare the write buffer. Fill in the data need to be written into
     * Flash Device.
     */
    for(i = 0; i < FLASH_TEST_SIZE; i++)
    {
		WriteBuffer[i] = i + 1;
    }

	// Write data to sector, then read out to compare
	for (secNum = startSec; secNum < endSec; secNum++)
	{
		TRACE_OUT(DEBUG_OUT, "NorFlash0 sector[%d] start program verify!\r\n", secNum);
		startAddr = NorFlash_GetDeviceSectorAddress(&pInstance->norFlash, secNum);
		
        // erase befor write
		ret = NORFLASH_EraseSector(&pInstance->norFlash, startAddr);
		if (ret != 0)
		{
			TRACE_OUT(DEBUG_OUT, "ERROR: Failed to erase sector[%d].\r\n", secNum);
			return FMSH_FAILURE;
		}
        
		data_cnt = secSize;
		while(data_cnt >= FLASH_TEST_SIZE)
		{
			// Perform the Write operation.
			ret = NORFLASH_WriteData(&pInstance->norFlash, startAddr, WriteBuffer, FLASH_TEST_SIZE);
			if (ret != 0)
			{
				TRACE_OUT(DEBUG_OUT, "ERROR: Failed to write NorFlash0 sector[%d].\r\n", secNum);
				return FMSH_FAILURE;
			}
			// Perform the Read operation.
			memset(ReadBuffer, 0, FLASH_TEST_SIZE * sizeof(u8));
			NORFLASH_ReadData(&pInstance->norFlash, startAddr, ReadBuffer, FLASH_TEST_SIZE);
			
			// Compare the data read against the data Written.
		    for(i = 0; i < FLASH_TEST_SIZE; i++)
		    {
		        if(ReadBuffer[i] != WriteBuffer[i])
		        {
	            	TRACE_OUT(DEBUG_OUT, "NorFlash0 sector[%d] Write/Read compare in data[0x%08x r:%x w:%x] failed!\r\n",
							  secNum , (startAddr + i), ReadBuffer[i], WriteBuffer[i]);
					data_comp |= 1;
					break;
		        }
		    }
			data_cnt = data_cnt - FLASH_TEST_SIZE;
			startAddr = startAddr + FLASH_TEST_SIZE;
		}

		if (data_cnt > 0)
		{
			// Perform the Write operation.
			ret = NORFLASH_WriteData(&pInstance->norFlash, startAddr, WriteBuffer, data_cnt);
			if (ret != 0)
			{
				TRACE_OUT(DEBUG_OUT, "ERROR: Failed to write NorFlash0 sector[%d].\r\n", secNum);
			}
			// Perform the Read operation.
			memset(ReadBuffer, 0, FLASH_TEST_SIZE * sizeof(u8));
			NORFLASH_ReadData(&pInstance->norFlash, startAddr, ReadBuffer, data_cnt);
			
			// Compare the data read against the data Written.
		    for(i = 0; i < data_cnt; i++)
		    {
		        if(ReadBuffer[i] != WriteBuffer[i])
		        {
	            	TRACE_OUT(DEBUG_OUT, "NorFlash0 sector[%d] Write/Read compare in data[0x%08x r:%x w:%x] failed!\r\n",
							  secNum, (startAddr + i), ReadBuffer[i], WriteBuffer[i]);
					data_comp |= 1;
					break;
		        }
		    }
		}
	}
	
	if (data_comp != 0)
		return FMSH_FAILURE;
	
    return FMSH_SUCCESS;
}

s32 FSmcPs_example(void)
{
	s32 status = FMSH_FAILURE;
	FSmcPs_Instance_T instance;

	FSmcPs_initDev(&g_SMC, &instance);
	
	TRACE_OUT(DEBUG_OUT, "Begin to run SMC0 test example.\r\n");
	TRACE_OUT(DEBUG_OUT, "/ -------\r\n");

	status = FSmcPs_progVerify(&g_SMC, 0, 5);

	return status;
}


