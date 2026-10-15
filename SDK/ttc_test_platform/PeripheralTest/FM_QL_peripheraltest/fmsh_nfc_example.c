
/***************************** Include Files *********************************/

#include <string.h>
#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_nfcps_lib.h"

/************************** Constant Definitions *****************************/

/*
* The following constants map to the XPAR parameters created in the
* xparameters.h file. They are defined here such that a user can easily
* change all the needed parameters in one place.
*/
#define NFC_DEVICE_ID		FPS_NFC_S_DEVICE_ID
#define INTC_DEVICE_ID		0
#define NFC_INTR_ID		    48

/*
* Flash address to which data is ot be written.
*/
#define TEST_ADDRESS		0x00090000
#define UNIQUE_VALUE		0x06

/*
* The following constants specify the max amount of data and the size of the
* the buffer required to hold the data and overhead to transfer the data to
* and from the FLASH.
*/
#define MAX_DATA		    2048

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

static FNfcPs_T nfc;
/******************************************************************************
*  @description: Erase, Write, Read function test
*
*
******************************************************************************/
int FNfcPs_example(void)
{
  int status;
  FNfcPs_T* nfcPtr;
  FNfcPs_Config_T* nfcConfig;
  u32 uniqueValue, count;
  u16 blockCount;
  
  nfcPtr = &nfc;
  /*
  * Initialize the QSPI driver so that it's ready to use
  */
  nfcConfig = FNfcPs_LookupConfig(FPS_NFC_S_DEVICE_ID);
  if (NULL == nfcConfig) 
  {
    return FMSH_FAILURE;
  }
  status = FNfcPs_CfgInitialize(nfcPtr, nfcConfig);
  if (status != FMSH_SUCCESS) 
  {
    return FMSH_FAILURE;
  }
  status = FNfcPs_InitController(nfcPtr, NFCPS_AUTO_READ_STAT_EN_MASK 
                                 //| NFCPS_READ_STATUS_EN_MASK
                                   );
  if(status != FMSH_SUCCESS)
  {
    return FMSH_FAILURE;
  }
  /*
  * Initialize the write buffer for a pattern to write to the FLASH
  * and the read buffer to zero so it can be verified after the read, the
  * test value that is added to the unique value allows the value to be
  * changed in a debug environment to guarantee
  */
  for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
  {
    WriteBuffer[count] = (u8)(uniqueValue);
    ReadBuffer[count] = 0;
    uniqueValue++;
  }
  
  /* 
  * erase
  */
  if(MAX_DATA % FNfcPs_GetBlockSizeInBytes(nfcPtr->model))
  {
    blockCount = (MAX_DATA / FNfcPs_GetBlockSizeInBytes(nfcPtr->model)) +1;
  }
  else
  {
    blockCount = MAX_DATA / FNfcPs_GetBlockSizeInBytes(nfcPtr->model);
  }
  FNfcPs_EraseBlock(nfcPtr, TEST_ADDRESS, blockCount);
  /* 
  * write data
  */
  FNfcPs_Write(nfcPtr, TEST_ADDRESS, MAX_DATA, WriteBuffer, 0);
  /* 
  * read data
  */
  FNfcPs_Read(nfcPtr, TEST_ADDRESS, MAX_DATA, ReadBuffer, 0);
  /*
  * Setup a pointer to the start of the data that was read into the read
  * buffer and verify the data read is the data that was written
  */
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
