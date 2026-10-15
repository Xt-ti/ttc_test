
/***************************** Include Files *********************************/

#include <string.h>
#include "fmsh_common.h"
#include "fmsh_gic.h"
#include "fmsh_slcr.h"
#include "fmsh_spips_lib.h"

/************************** Constant Definitions *****************************/

/*
* The following constants map to the XPAR parameters created in the
* xparameters.h file. They are defined here such that a user can easily
* change all the needed parameters in one place.
*/
#define SPI_DEVICE_ID		FPS_SPI0_DEVICE_ID
#define INTC_DEVICE_ID		0
#define SPI_INTR_ID		    SPI0_INTR_ID

/*
* Flash data is to be written.
*/
#define UNIQUE_VALUE		0x05

/*
* The following constants specify the max amount of data and the size of the
* the buffer required to hold the data and overhead to transfer the data to
* and from the FLASH.
*/
#define MAX_DATA		32

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int FSpiPs_setupIntrSystem(FGicPs* intcPtr, FSpiPs_T* spiPtr, u16 spiIntrId);
static void FSpiPs_disableIntrSystem(FGicPs* intcPtr, u16 spiIntrId);
static void FSpiPs_handler(void *callBackRef, u32 statusEvent, u32 byteCount);

/************************** Variable Definitions *****************************/
/*
* The following variables are used to read and write to the flash and they
* are global to avoid having large buffers on the stack
*/
static u8 ReadBuffer[MAX_DATA];
static u8 WriteBuffer[MAX_DATA];

static FSpiPs_T spi0, spi1;

int SpiPolledExample(FSpiPs_T* spiPtr, u16 spiDeviceId)
{
    int status;
    FSpiPs_Config_T* spiConfig;
    u32 uniqueValue, count;  
    
    /*
    * Initialize the SPI driver so that it's ready to use
    */
    spiConfig = FSpiPs_LookupConfig(spiDeviceId);
    if (NULL == spiConfig) 
    {
        return FMSH_FAILURE;
    }
    status = FSpiPs_CfgInitialize(spiPtr, spiConfig);
    if (status != FMSH_SUCCESS) 
    {
        return FMSH_FAILURE;
    }
  
    status = FSpiPs_Initialize_Master(spiPtr);
    if(status != FMSH_SUCCESS)
    {
        return FMSH_FAILURE;
    }
    
    /*enable selfloop mode*/
    FSpiPs_Disable(spiPtr);
    FSpiPs_SetLoopBack(spiPtr, TRUE);
    FSpiPs_Enable(spiPtr); 
    
    /*
    * Initialize the write buffer for a pattern to write to the FLASH
    * and the read buffer to zero so it can be verified after the read, the
    * test value that is added to the unique value allows the value to be
    * changed in a debug environment to guarantee
    */
    for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
    {
        WriteBuffer[count] = (u8)(uniqueValue);
        uniqueValue++;
    }
            
    FSpiPs_PolledTransfer(spiPtr, WriteBuffer, ReadBuffer, MAX_DATA);
    
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

int FSpiPs_intrExample(FGicPs* intcPtr, FSpiPs_T* spiPtr, 
                   u16 spiDeviceId, u16 spiIntrId)
{
  int status;
  FSpiPs_Config_T* spiConfig;
  u32 uniqueValue, count;  
  int time = 0;
  /*
  * Initialize the QSPI driver so that it's ready to use
  */
  spiConfig = FSpiPs_LookupConfig(spiDeviceId);
  if (NULL == spiConfig) 
  {
    return FMSH_FAILURE;
  }
  status = FSpiPs_CfgInitialize(spiPtr, spiConfig);
  if (status != FMSH_SUCCESS) 
  {
    return FMSH_FAILURE;
  }
  status = FSpiPs_Initialize_Master(spiPtr);
  if(status != FMSH_SUCCESS)
  {
    return FMSH_FAILURE;
  }
  FSpiPs_SetLoopBack(spiPtr, TRUE);
  
  FSpiPs_setupIntrSystem(intcPtr, spiPtr, spiIntrId);
  
  /*
  * Initialize the write buffer for a pattern to write to the FLASH
  * and the read buffer to zero so it can be verified after the read, the
  * test value that is added to the unique value allows the value to be
  * changed in a debug environment to guarantee
  */
  for (uniqueValue = UNIQUE_VALUE, count = 0; count < MAX_DATA; count++)
  {
    WriteBuffer[count] = (u8)(uniqueValue);
    uniqueValue++;
  }
  
  FSpiPs_Transfer(spiPtr, WriteBuffer, ReadBuffer, MAX_DATA);
  while(spiPtr->isBusy == TRUE)
  {
    delay_ms(10);
    time++;
    if(time > 500)
    {
      spiPtr->isBusy = FALSE;
      break;
    }      
  }
  
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
  
  FSpiPs_disableIntrSystem(intcPtr, spiIntrId);
  
  return FMSH_SUCCESS;  
}

/*****************************************************************************/
/**
*
* This function setups the interrupt system for an Qspi device.
*
* @param	intcPtr is a pointer to the instance of the Intc device.
* @param	QspiInstancePtr is a pointer to the instance of the Qspi device.
* @param	QspiIntrId is the interrupt Id for an QSPI device.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FSpiPs_setupIntrSystem(FGicPs* intcPtr, FSpiPs_T* spiPtr, u16 spiIntrId)
{
  int status;
  FGicPs_Config* intcConfig; /* Instance of the interrupt controller */
  
  /*
  * Initialize the interrupt controller driver so that it is ready to
  * use.
  */
  intcConfig = FGicPs_LookupConfig(INTC_DEVICE_ID);
  if (NULL == intcConfig) 
  {
    return FMSH_FAILURE;
  }
  
  status = FGicPs_CfgInitialize(intcPtr, intcConfig, intcConfig->CpuBaseAddress);
  if (status != FMSH_SUCCESS) 
  {
    return FMSH_FAILURE;
  }
  
  /*
  * Connect the device driver handler that will be called when an
  * interrupt for the device occurs, the handler defined above performs
  * the specific interrupt processing for the device.
  */
  status = FGicPs_Connect(intcPtr, spiIntrId, (FMSH_InterruptHandler)FSpiPs_handler,
                          (void *)spiPtr);
  if (status != FMSH_SUCCESS) {
    return status;
  }
  
  /*
  * Enable the interrupt for the Qspi device.
  */
  FGicPs_Enable(intcPtr, spiIntrId);
  
  return FMSH_SUCCESS;    
}

/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the Qspi device.
*
* @param	intcPtr is the pointer to an INTC instance.
* @param	QspiIntrId is the interrupt Id for an QSPI device.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void FSpiPs_disableIntrSystem(FGicPs* intcPtr, u16 spiIntrId)
{
  /*
  * Disable the interrupt for the QSPI device.
  */
  FGicPs_Disable(intcPtr, spiIntrId);
  
  /*
  * Disconnect and disable the interrupt for the Qspi device.
  */
  FGicPs_Disconnect(intcPtr, spiIntrId);
}

/*****************************************************************************/
/**
*
* This function disables the interrupts that occur for the Qspi device.
*
* @param	intcPtr is the pointer to an INTC instance.
* @param	QspiIntrId is the interrupt Id for an QSPI device.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void FSpiPs_handler(void *callBackRef, u32 statusEvent, u32 byteCount)
{
  FSpiPs_T* spiPtr = (FSpiPs_T*)callBackRef;
  
  if(statusEvent == SPIPS_INTR_MSTIS_MASK)
  {
    
  }
  if(statusEvent == SPIPS_INTR_RXOIS_MASK)
  {
    
  }
  if(statusEvent == SPIPS_INTR_RXUIS_MASK)
  {
    
  }    
  if(statusEvent == SPIPS_INTR_TXOIS_MASK)
  {
    
  }        
  if(statusEvent == SPIPS_INTR_TXEIS_MASK)
  {
    /* One transmit completed, check Rx FIFO */
    while((spiPtr->requestedBytes>0) &&
          (FSpiPs_GetStatus(spiPtr) & SPIPS_SR_RFNE))
    { 
      if(spiPtr->frameSize == 8)
      {
        *(u8*)(spiPtr->recvBufferPtr) = (u8)FSpiPs_Recv(spiPtr);
      }
      else if(spiPtr->frameSize == 16)
      {
        *(u16*)(spiPtr->recvBufferPtr) = (u16)FSpiPs_Recv(spiPtr);
      }
      else if(spiPtr->frameSize == 32)
      {
        *(u32*)(spiPtr->recvBufferPtr) = (u32)FSpiPs_Recv(spiPtr);
      }
      
      spiPtr->recvBufferPtr += spiPtr->frameSize >> 3;
      spiPtr->requestedBytes -= spiPtr->frameSize >> 3;
    }
    u32 transCount=0;
    /* check if more data wait transfer */
    while((spiPtr->remainingBytes > 0) &&
          (FSpiPs_GetStatus(spiPtr) & SPIPS_SR_TFNF) &&
            (transCount < SPIPS_FIFO_DEPTH))
    {
      
      if(spiPtr->frameSize == 8)
      {
        FSpiPs_Send(spiPtr, *(u8*)spiPtr->sendBufferPtr);
      }
      else if(spiPtr->frameSize == 16)
      {
        FSpiPs_Send(spiPtr, *(u16*)spiPtr->sendBufferPtr);
      }
      else if(spiPtr->frameSize == 32)
      {
        FSpiPs_Send(spiPtr, *(u32*)spiPtr->sendBufferPtr);
      }
      spiPtr->sendBufferPtr += spiPtr->frameSize >> 3;
      spiPtr->remainingBytes -= spiPtr->frameSize >> 3;
      transCount++;
    }
  }
  if(statusEvent == SPIPS_INTR_RXFIS_MASK)
  {
    /* One transmit completed, check Rx FIFO */
    while((spiPtr->requestedBytes>0) &&
          (FSpiPs_GetStatus(spiPtr) & SPIPS_SR_RFNE))
    {
      if(spiPtr->frameSize == 8)
      {
        *(u8*)(spiPtr->recvBufferPtr) = (u8)FSpiPs_Recv(spiPtr);
      }
      else if(spiPtr->frameSize == 16)
      {
        *(u16*)(spiPtr->recvBufferPtr) = (u16)FSpiPs_Recv(spiPtr);
      }
      else if(spiPtr->frameSize == 32)
      {
        *(u32*)(spiPtr->recvBufferPtr) = (u32)FSpiPs_Recv(spiPtr);
      }
      spiPtr->recvBufferPtr += spiPtr->frameSize >> 3;
      spiPtr->requestedBytes -= spiPtr->frameSize >> 3;
    }
  } 
  if(statusEvent == SPIPS_TRANSFER_DONE)
  {
    FSpiPs_DisableIntr(spiPtr, SPIPS_INTR_ALL);
  }   
}

/*****************************************************************************/
/**
*
* This function use low level function to transfer data from spi0 to spi1.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FSpiPs_example(void)
{
    FSpiPs_T* spi0Ptr;
    FSpiPs_T* spi1Ptr;
    int status = FMSH_SUCCESS;
    u32 data;
    
    spi0Ptr = &spi0;
    spi1Ptr = &spi1;
    
    /* enable loop */
    FSlcrPS_setSpiLoop(TRUE);
    
    /* spi0 master, spi1 slaver */
    FSpiPs_Initialize(spi0Ptr, FPS_SPI0_DEVICE_ID);
    FSpiPs_Initialize(spi1Ptr, FPS_SPI1_DEVICE_ID);
    FSpiPs_Reset(spi0Ptr);
    FSpiPs_Reset(spi1Ptr);
    FSpiPs_Initialize_Master(spi0Ptr);
    FSpiPs_Initialize_Slave(spi1Ptr);   

    /* slave sends first, then master sends */
    FSpiPs_Send(spi1Ptr, 0x66);
    FSpiPs_Send(spi0Ptr, 0xaa); 
    delay_ms(1);
    data = FMSH_ReadReg(spi0Ptr->config.baseAddress, SPIPS_DR_OFFSET);
    if(data != 0x66) 
        status = FMSH_FAILURE;
    data = FMSH_ReadReg(spi1Ptr->config.baseAddress, SPIPS_DR_OFFSET);
    if(data != 0xaa) 
        status = FMSH_FAILURE;  
    
    /* disable loop */
    FSlcrPS_setSpiLoop(FALSE);
    
    return status;
}