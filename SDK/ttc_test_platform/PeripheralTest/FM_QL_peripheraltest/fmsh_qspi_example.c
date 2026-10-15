
/***************************** Include Files *********************************/

#include <string.h>
#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_qspips_lib.h"

/************************** Constant Definitions *****************************/

/*
* The following constants map to the XPAR parameters created in the
* xparameters.h file. They are defined here such that a user can easily
* change all the needed parameters in one place.
*/
#define QSPI_DEVICE_ID		FPS_QSPI0_DEVICE_ID
#define INTC_DEVICE_ID		0
#define QSPI_INTR_ID		46

/*
* Flash address to which data is ot be written.
*/
#define TEST_ADDRESS		0x00090000
#define UNIQUE_VALUE        5

/*
* The following constants specify the max amount of data and the size of the
* the buffer required to hold the data and overhead to transfer the data to
* and from the FLASH.
*/
#define MAX_DATA		4096

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*
* The following variables are used to read and write to the flash and they
* are global to avoid having large buffers on the stack
*/
static u8 ReadBuffer[MAX_DATA];
static u8 WriteBuffer[MAX_DATA];

static FQspiPs_T qspi;

/******************************************************************************
* @description: direct mode(x1, x2, x4) verify
* 
*
*
******************************************************************************/
int FQspiPs_example(void)
{
  int status;
  FQspiPs_T* qspiPtr;
  FQspiPs_Config_T* qspiConfig;
  u32 count, uniqueValue;
  
  qspiPtr = &qspi;
  /*
  * Initialize the QSPI driver so that it's ready to use
  */
  qspiConfig = FQspiPs_LookupConfig(FPS_QSPI0_DEVICE_ID);
  if (NULL == qspiConfig) 
  {
    return FMSH_FAILURE;
  }
  
  status = FQspiPs_CfgInitialize(qspiPtr, qspiConfig);
  if (status != FMSH_SUCCESS) 
  {
    return FMSH_FAILURE;
  }
  qspiPtr->sectorSize = QSPIFLASH_SECTOR_256K;
  
  /* 
  * reset controller
  */
  FQspiPs_Reset(qspiPtr); 
  
  /*
  * Perform a self-test to check hardware build
  */
  status = FQspiPs_SelfTest(qspiPtr);
  if (status != FMSH_SUCCESS) 
  {
    return FMSH_FAILURE;
  }    
  
  /* get maker & device size */
  status = FQspiPs_GetFlashInfo(qspiPtr);
  if(status != FMSH_SUCCESS)
  {
    return FMSH_FAILURE;
  } 	 
  
  /* 
  * set baud rate at 4 
  */
  FQspiPs_SetBaudRate(qspiPtr, QSPIPS_BAUD(8));
  
  /*
  * set transfer mode 
  */
  status = FQspiPs_SetFlashMode(qspiPtr, QOR_CMD); 
  if(status != FMSH_SUCCESS)
  {
    return FMSH_FAILURE;
  } 	
  
  for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
  {
    WriteBuffer[count] = (u8)(uniqueValue);
    uniqueValue++;
  }
  /*
  * Erase data
  */
  FQspiPs_UnlockFlash(qspiPtr);
  qspiPtr->sectorSize = (0x1 << 16);
  FQspiPs_EraseSectors(qspiPtr, TEST_ADDRESS, MAX_DATA, qspiPtr->sectorSize);
  /*
  * Write & Read data
  */
  FQspiPs_SendBytes(qspiPtr, TEST_ADDRESS, MAX_DATA, WriteBuffer);
  FQspiPs_RecvBytes(qspiPtr, TEST_ADDRESS, MAX_DATA, ReadBuffer);
  
  for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++) 
  {
    if (ReadBuffer[count] != (u8)(uniqueValue))
    {
      return FMSH_FAILURE;
    }
    uniqueValue++;
  }
  
  return FMSH_SUCCESS;
}