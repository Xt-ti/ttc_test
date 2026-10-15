/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/******************************************************************************
*
* @file  fmsh_i2c_example.c
*
* This file contains a example of i2c.
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   LQ  11/23/2018  First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "fmsh_i2c_example.h"

#include "fmsh_gic.h"


/************************** Constant Definitions *****************************/
#define SLAVE_ADDRESS 0x5A
#define EEPROM_ADDR   0x50
#define I2C_BUF_LEN   10
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
FI2cPs_T gI2c0_dev;
FI2cPs_T gI2c1_dev;
FIicPs_Instance_T gI2c0_Instance;
FIicPs_Instance_T gI2c1_Instance;

static volatile bool s_i2cMasterWaitFlag;

static volatile bool s_i2cSlaveWaitFlag;

static uint8_t s_i2cMasterTxBuffer[I2C_BUF_LEN];

static uint8_t s_i2cSlaveRxBuffer[I2C_BUF_LEN];

/************************** Function Prototypes ******************************/

void iic0_interrupt_handler(void *InstancePtr)
{
  FI2cPs_irqHandler(&gI2c0_dev);
}

void iic1_interrupt_handler (void *InstancePtr)
{ 
   FI2cPs_irqHandler(&gI2c1_dev);
}

void user_iic0_interrupt_handler(void *InstancePtr)
{
  FI2cPs_userIrqHandler(&gI2c0_dev);
}

void user_iic1_interrupt_handler (void *InstancePtr)
{ 
  FI2cPs_userIrqHandler(&gI2c1_dev);
}

void FI2cPs_MasterCallback(void *dev,int32_t numBytes)
{
  s_i2cMasterWaitFlag = false;
}

void FI2cPs_SlaveCallback(void *dev,int32_t numBytes)
{
  s_i2cSlaveWaitFlag = false;
}

void FI2cPs_MasterListener(void *device, int32_t ecode)
{
    // variable to store the reason(s) for transfer aborts
    FIicPs_TxAbort_T txAbort;
    FI2cPs_T* dev=device;
    // Note that the I2c_irq_tx_empty and I2c_irq_rx_done interrupts are
    // always handled internally by the driver and are never passed to
    // the listener function.  Also note that the interrupt handler is
    // responsible for the clearing of all interrupts, which it does
    // after calling the user listener function.
    switch(ecode) {
        case I2c_irq_rx_under:
            // Master receiver FIFO underflow.
            printf("*** Master: Rx FIFO underflow ***\n");
            //FMSH_ASSERT(false);
            break;
        case I2c_irq_rx_over:
            // Master receiver FIFO overflow.
            printf("*** Master: Rx FIFO overflow ***\n");
            //FMSH_ASSERT(false);
            break;
        case I2c_irq_rx_full:
             //i2c0_rxBuffer_idx++;
             //I2C0_RxBuffer[i2c0_rxBuffer_idx] = i2c_read(dev);
              //i2c_masterReceive(dev,I2C_Master_RxBuffer,10,test_i2c_master_callback);
            break;
        case I2c_irq_tx_over:
            // Master transmitter FIFO overflow.
            printf("*** Master: Tx FIFO overflow ***\n");
            //FMSH_ASSERT(false);
            break;
        case I2c_irq_rd_req:
            // In this example, the master device slave FSM is disabled.
            // Therefore, this interrupt should never be triggered as
            // the master device initiates all transfers.
            printf("\n*** Master: unexpected read request ***\n");
           // FMSH_ASSERT(false);
            break;
        case I2c_irq_tx_abrt:
            // Find out the reason for the transfer abort.
            txAbort = FI2cPs_getTxAbortSource(dev);
            // Print an error message with the value of the
            // tx_abort_source register.
            printf("*** Master tx abort: 0x%04x ***\n", (uint16_t)
                    txAbort);
            break;
        case I2c_irq_activity:
            // This interrupt is disabled by default by the driver
            // (in the i2c_setListener function).  It is triggered
            // whenever there is activity on the I2C bus.
            break;
        case I2c_irq_stop_det:
            // This interrupt is disabled by default by the driver
            // (in the i2c_setListener function).  It is triggered
            // whenever a stop condition is detected on the I2C bus.
            break;
        case I2c_irq_start_det:
            // This interrupt is disabled by default by the driver
            // (in the i2c_setListener function).  It is triggered
            // whenever a start condition is detected on the I2C bus.
            break;
        case I2c_irq_gen_call:
            // In this example, the master device slave FSM is disabled.
            // Therefore, this interrupt should never be triggered as
            // the master device initiates all transfers.
            printf("\n*** Master: unexpected general call ***\n");
           // FMSH_ASSERT(false);
            break;
        case I2c_scl_stuck_at_low:
          //Abort tx, issue stop
            FI2cPs_masterAbort(dev);
            break;
        default:
            // Print an error message if an unexpected value is passed
            // as the argument to this function.
            printf("*** unexpected argument: 0x%x ***\n", ecode);
            printf("*** disabling master device... ");
            while(FI2cPs_disable(dev) == -FMSH_EBUSY);
            printf("***\n");
            //FMSH_ASSERT(false);
            break;
    }
}

// A user listener function for the slave device.
void FI2cPs_SlaveListener(void *device, int32_t ecode)
{
    FIicPs_TxAbort_T txAbort;
//    FIicPs_Instance_T *instance;
//    u16 rxfifolvl;
//    u16 i;
    FI2cPs_T *dev=device;
//    instance = (FIicPs_Instance_T *) dev->instance;
//   pBuffer = instance->rxBuffer;

    // Note that the I2c_irq_tx_empty and I2c_irq_rx_done interrupts are
    // always handled internally by the driver and are never passed to
    // the listener function.  Also note that the interrupt handler is
    // responsible for the clearing of all interrupts, which it does
    // after calling the user listener function.
    switch(ecode) {
        case I2c_irq_rx_under:
            // Slave receiver FIFO underflow.
            printf("*** Slave Rx FIFO underflow ***\n");
            break;
        case I2c_irq_rx_over:
            // Slave receiver FIFO overflow.
            printf("*** Slave Rx FIFO overflow ***\n");
            break;
        case I2c_irq_rx_full:
            // In this example, the slave device is configured to use
            // the 20-byte rx_buffer for all receive transfers.  The
            // Rx full interrupt is passed to the user listener function
            // only if there is no receive buffer already set up to
            // accept data.  The example slave callback function is
            // called when 'master_writes' bytes have been received.
          
          // i2c_slaveReceive(dev, rx_buffer, master_writes,dwe_i2c_slave_callback);
           //i2c_slaveReceive(dev, I2C_Slave_RxBuffer, 10, test_i2c_slave_callback);
          
//            rxfifolvl = FI2cPs_getRxFifoLevel(dev);
//            for(i=0;i<rxfifolvl;i++)
//            {
//              s_i2cSlaveRxBuffer[i] = FI2cPs_read(dev);
//            }
 
            //instance->state = I2c_state_idle;
        
            break;
        case I2c_irq_tx_over:
            // Slave transmitter FIFO overflow.
            printf("*** Slave: Tx FIFO overflow ***\n");
            break;
        case I2c_irq_rd_req:
            // In this example, the slave device is configured to use
            // the 20-byte tx_buffer for all transmit transfers.  No
            // callback function is called at the end of the transfer as
            // none is specified (i.e. argument is NULL).
//             i2c_slaveBulkTransmit(dev, I2C_Slave_TxBuffer, 10, NULL);
          
            break;
        case I2c_irq_tx_abrt:
            // Find out the reason for the transfer abort.
            txAbort = FI2cPs_getTxAbortSource(dev);
            // Print an error message with the value of the
            // tx_abort_source register.
            TRACE_OUT(DEBUG_OUT, "*** Slave tx abort: 0x%04x ***\n", (uint16_t)
                    txAbort);
            break;
        case I2c_irq_activity:
            // This interrupt is disabled by default by the driver
            // (in the i2c_setListener function).  It is triggered
            // whenever there is activity on the I2C bus.
            break;
        case I2c_irq_stop_det:
            // This interrupt is disabled by default by the driver
            // (in the i2c_setListener function).  It is triggered
            // whenever a stop condition is detected on the I2C bus.
            break;
        case I2c_irq_start_det:
            // This interrupt is disabled by default by the driver
            // (in the i2c_setListener function).  It is triggered
            // whenever a start condition is detected on the I2C bus.
            break;
        case I2c_irq_gen_call:
            // In this example, the slave device is configured to use
            // the 20-byte rx_buffer for all receive transfers.  The
            // example slave callback function is called when
            // 'master_writes' bytes have been received.
            
            break;
        default:
            // Print an error message if an unexpected value is passed
            // as the argument to this function.
            printf("*** unexpected argument: 0x%x ***\n", ecode);
            printf("*** disabling slave device... ");
            while(FI2cPs_disable(dev) == -FMSH_EBUSY);
            printf("***\n");
            break;
    }
}

u8 FI2c0Ps_DeviceInit(
                      FI2cPs_T *pDev,
                      void* pI2cInstance,
                      void* I2cParam )
{
    u8 ret=FMSH_SUCCESS;     
    FI2cPs_Config* Config=NULL;
    Config = FI2cPs_LookupConfig(FPAR_I2CPS_0_DEVICE_ID);
    if(Config==NULL)
      return FMSH_FAILURE;
    ret=FI2cPs_init(pDev, Config,pI2cInstance,I2cParam);
    if(ret!=FMSH_SUCCESS)
      return ret;
    return ret;
}

u8 FI2c1Ps_DeviceInit(
                      FI2cPs_T *pDev,
                      void* pI2cInstance,
                      void* I2cParam )
{
    u8 ret=FMSH_SUCCESS;     
    FI2cPs_Config* Config=NULL;
    Config = FI2cPs_LookupConfig(FPAR_I2CPS_1_DEVICE_ID);
    if(Config==NULL)
      return FMSH_FAILURE;
    ret=FI2cPs_init(pDev, Config,pI2cInstance,I2cParam);
    if(ret!=FMSH_SUCCESS)
      return ret;
    return ret;
}

void FI2cPs_MasterInit(FI2cPs_T *dev)
{
    /*disable the dev I2C device*/
    FI2cPs_disable(dev);
    
    FIicPs_PortMap_T *pPortmap = (FIicPs_PortMap_T *)dev -> base_address;
 
    /* Config  */  
    I2C_OUTP(0x1,pPortmap->fs_spklen);  //offset = 0xA0 IC_FS_SPKLEN
    //FMSH_OUT32_32(0x5,pPortmap->reserved1);
    
    /* Set up the clock count register.  The argument I2C1_CLOCK is specified as the I2C dev input clock.*/
    FI2cPs_ClockSetup(dev, (dev->input_clock)/1000000);
    
    /* set the speed mode to standard*/
    FI2cPs_setSpeedMode(dev, I2c_speed_standard);
    
    /* use 7&10-bit addressing*/
    FI2cPs_setMasterAddressMode(dev, I2c_7bit_address);
    FI2cPs_setSlaveAddressMode(dev, I2c_7bit_address);
    
    /* enable restart conditions*/
    FI2cPs_enableRestart(dev);
    
    /* enable master FSM*/
    FI2cPs_enableMaster(dev);
    
    /* Use the start byte protocol with the target address when initiating transfer.*/
    FI2cPs_setTxMode(dev, I2c_tx_target);
 
    /* set target address to the I2C slave address*/
    FI2cPs_setTargetAddress(dev, SLAVE_ADDRESS);
    
    /*Set the user listener function*/
    FI2cPs_setListener(dev, FI2cPs_MasterListener);

    /* clear Irq */
    FI2cPs_clearIrq(dev,I2c_irq_all);
    
    /*enable the dev I2C device*/
   //i2c_enable(dev);
    
}

void FI2cPs_SlaveInit(FI2cPs_T *dev)
{
    /*disable the dev I2C device*/
    FI2cPs_disable(dev);
  
    FIicPs_PortMap_T *pPortmap = (FIicPs_PortMap_T *)dev -> base_address;
 
    /* Config  */
    I2C_OUTP(0x1D7,pPortmap->reserved3[12]);  //offset = C4
    
    /* Set up the clock count register.  The argument I2C_CLOCK is specified as the I2C dev input clock.*/
    FI2cPs_ClockSetup(dev, (dev->input_clock)/1000000);
    
    /* set the speed mode to standard*/
    FI2cPs_setSpeedMode(dev, I2c_speed_standard);
    
    /* use 7&10-bit addressing*/
    FI2cPs_setMasterAddressMode(dev, I2c_7bit_address);
    FI2cPs_setSlaveAddressMode(dev, I2c_7bit_address);
   
    /* enable restart conditions*/
    FI2cPs_enableRestart(dev);
    
    /* enable slave FSM*/
    FI2cPs_enableSlave(dev);
    FI2cPs_disableMaster(dev);
    
    /* set target address to the I2C slave address*/
    FI2cPs_setSlaveAddress(dev, SLAVE_ADDRESS);
    
    /*Set the user listener function*/
    FI2cPs_setListener(dev, FI2cPs_SlaveListener);
    
    /* clear Irq */
    FI2cPs_clearIrq(dev,I2c_irq_all);
    
}

/******************************************************************************
*
* @description
*    A example of i2c, send data.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FI2cPs_example(void)
{
  u8 Status=0;
  u8 i=0;
  
  FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x008, 0xDF0D767BU);//unlock    
        
        FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x804, 0x8);//loop back  
        FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x314, 0x3);//loop back  
       
        FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x004, 0xDF0D767BU);//lock  
  FI2cPs_T *pI2c0_dev = &gI2c0_dev;
  FIicPs_Instance_T *pI2c0_Instance=&gI2c0_Instance;
  FIicPs_Param_T I2c0_Param;
  
  FI2cPs_T *pI2c1_dev = &gI2c1_dev;
  FIicPs_Instance_T *pI2c1_Instance=&gI2c1_Instance;
  FIicPs_Param_T I2c1_Param;
  
  Status = FGicPs_Connect(&IntcInstance, I2C0_INT_ID,
                   (FMSH_InterruptHandler)iic0_interrupt_handler, 0);
  
  if (Status != GIC_SUCCESS)
  {
    TRACE_OUT(DEBUG_OUT, "Failed to generate I2C0 int.\r\n");
  }
  FGicPs_Enable(&IntcInstance, I2C0_INT_ID);

  Status = FGicPs_Connect(&IntcInstance, I2C1_INT_ID,
                   (FMSH_InterruptHandler)iic1_interrupt_handler, 0);
  
  if (Status != GIC_SUCCESS)
  {
    TRACE_OUT(DEBUG_OUT, "Failed to generate I2C1 int.\r\n");
  }
  FGicPs_Enable(&IntcInstance, I2C1_INT_ID);
  
  for(i=0;i<10;i++)
  { 
    s_i2cMasterTxBuffer[i] = i*2+1;
    s_i2cSlaveRxBuffer[i] = 0;
  }
  
  /* Initial I2C0 */
  FI2c0Ps_DeviceInit(pI2c0_dev,pI2c0_Instance,&I2c0_Param);
  FI2cPs_MasterInit(pI2c0_dev);    //In default config the device as master

  /* Initial I2C1 */
  FI2c1Ps_DeviceInit(pI2c1_dev,pI2c1_Instance,&I2c1_Param);
  FI2cPs_SlaveInit(pI2c1_dev);    //In default config the device as slave

  FI2cPs_enable(pI2c1_dev);

  FI2cPs_enable(pI2c0_dev);
  
  /* Test Master-Transmitter & Master-Receiver */
  s_i2cMasterWaitFlag = s_i2cSlaveWaitFlag = true;
  
  /* Initiate the transfer. A callback function will be called when the
    last byte has been transmitted by the master device. */
  TRACE_OUT(DEBUG_OUT, "Issue Master-Transmit...\n");
  
  FI2cPs_slaveReceive(pI2c1_dev, s_i2cSlaveRxBuffer, 10, FI2cPs_SlaveCallback);
  FI2cPs_masterTransmit(pI2c0_dev,s_i2cMasterTxBuffer,10,FI2cPs_MasterCallback ); 
 
//  FI2cPs_slaveReceive(pI2c1_dev, s_i2cSlaveRxBuffer, 10, FI2cPs_SlaveCallback);
  
  /* Wait until Master has finished the transfer */
  while(s_i2cMasterWaitFlag == true);
  TRACE_OUT(DEBUG_OUT, "Master-Transmit is done.\n");
  s_i2cMasterWaitFlag = true;

  /* Wait until the transfer has finished */
  /* The Slave_listener automatically responds to an Rx FIFO full Irq*/
  while(s_i2cSlaveWaitFlag == true);
  TRACE_OUT(DEBUG_OUT, "Slave-Receive is done.\n");  
  s_i2cSlaveWaitFlag = true;
  
  for(i =  0; i<10;i++)
  {
    if(s_i2cMasterTxBuffer[i] != s_i2cSlaveRxBuffer[i])
     return FMSH_FAILURE;
  }
  return FMSH_SUCCESS;
}


u8 FI2cPs_EepromInit(FI2cPs_T *dev)
{
    /* Set up the clock count register.  The argument I2C1_CLOCK is
     specified as the I2C dev input clock.*/
    FI2cPs_ClockSetup(dev, (dev->input_clock)/1000000);
    /* set the speed mode to standard*/
    FI2cPs_setSpeedMode(dev, I2c_speed_standard);
    /* use 7-bit addressing*/
    FI2cPs_setMasterAddressMode(dev, I2c_7bit_address);
    FI2cPs_setSlaveAddressMode(dev, I2c_7bit_address);
    /* enable restart conditions*/
    FI2cPs_enableRestart(dev);
    /* enable master FSM*/
    FI2cPs_enableMaster(dev);
    FI2cPs_disableSlave(dev);
    
    // Use the start byte protocol with the target address when
    // initiating transfer.
    FI2cPs_setTxMode(dev, I2c_tx_target);
    
    /* set target address to the I2C slave address*/
    FI2cPs_setTargetAddress(dev, EEPROM_ADDR);
        
    /*enable the dev I2C device*/
    FI2cPs_enable(dev); 
    
    return 0;
}

void FI2cPs_EepromWrite(FI2cPs_T *master,uint16_t iaddress,uint8_t  byte)
{
    uint8_t bHaddr,bLaddr;
    
    bHaddr = (uint8_t)(iaddress >> 8);
    bLaddr = (uint8_t)iaddress;
   
    /* write internal address */
    FI2cPs_write(master,bHaddr);
    FI2cPs_write(master,bLaddr);
    
    FI2cPs_issueSTOP(master,byte);
}

uint8_t FI2cPs_EepromRead( FI2cPs_T *master,uint16_t iaddress)
{
    uint8_t bHaddr,bLaddr;
    uint8_t bRtval = 0;
    //uint16_t fifo_level=0;
    
    bHaddr = (uint8_t)(iaddress >> 8);
    bLaddr = (uint8_t)iaddress;
    
    /*dummy write*/
    FI2cPs_write(master,bHaddr);
    FI2cPs_write(master,bLaddr);
    /*Issue read*/
    FI2cPs_issueRead(master);
    
    while(FI2cPs_isRxFifoEmpty(master) == true);

    bRtval = FI2cPs_read(master);
    return bRtval;
       
}

/******************************************************************************
*
* @description
*    A example of i2c write and read data from the eeprom.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FI2cPs_example2(void)
{
  u8 i=0;
  
  FI2cPs_T *pI2c1_dev = &gI2c1_dev;
  FIicPs_Instance_T *pI2c1_Instance=&gI2c1_Instance;
  FIicPs_Param_T I2c1_Param;
  
  FI2c1Ps_DeviceInit(pI2c1_dev,pI2c1_Instance,&I2c1_Param);
  FI2cPs_EepromInit(pI2c1_dev);
  
  for(i=0;i<10;i++)
    s_i2cMasterTxBuffer[i] = i*2+1;


  
  for(i=0;i<10;i++)
  {
    //Write
    FI2cPs_EepromWrite(pI2c1_dev,i,s_i2cMasterTxBuffer[i]); 
    delay_ms(20);
    //Read  
    if(FI2cPs_EepromRead(pI2c1_dev,i)!=s_i2cMasterTxBuffer[i])
       break;
  }

  FI2cPs_masterAbort(pI2c1_dev);
  if(i == 10)
      return 0;
  else
      return 1;
     
  
}



u32 FI2cPs_ReadNRS1800Reg(FI2cPs_T *master,u16 ReadNum,u32 MemoryAddr)
{
    
    u32 reg=0; 
    /* --POLL-- */
    /*
    FI2cPs_write(master,0x80|(ReadNum>>2));
    FI2cPs_write(master,(ReadNum<<6)|(MemoryAddr>>18));
    FI2cPs_write(master,(ReadNum<<6)|(MemoryAddr>>10));
    FI2cPs_write(master,(ReadNum<<6)|(MemoryAddr>>2));
    FI2cPs_enableRestart(master);
    FI2cPs_issueRead(master);
    reg=FI2cPs_read(master);
    reg<<=8;
    reg=FI2cPs_read(master);
    reg<<=8;
    reg=FI2cPs_read(master);
    reg<<=8;
    reg=FI2cPs_read(master);
    */
    /* --INT-- */
    s_i2cMasterWaitFlag = true;
    s_i2cMasterTxBuffer[0]=0x80|(ReadNum>>2);
    s_i2cMasterTxBuffer[1]=(ReadNum<<6)|(MemoryAddr>>18);
    s_i2cMasterTxBuffer[2]=(ReadNum<<6)|(MemoryAddr>>10);
    s_i2cMasterTxBuffer[2]=(ReadNum<<6)|(MemoryAddr>>2);
    FI2cPs_masterTransmit(master,s_i2cMasterTxBuffer,4,FI2cPs_MasterCallback); 
    while(s_i2cMasterWaitFlag == true);
    FI2cPs_enableRestart(master);
    s_i2cMasterWaitFlag = true;
    FI2cPs_masterReceive(master,s_i2cSlaveRxBuffer,4, FI2cPs_MasterCallback);
    while(s_i2cMasterWaitFlag == true);
    reg=(s_i2cSlaveRxBuffer[0]<<24)|(s_i2cSlaveRxBuffer[1]<<16)|(s_i2cSlaveRxBuffer[2]<<8)|s_i2cSlaveRxBuffer[3];
    
    FI2cPs_disableRestart(master);
    return reg;
}

/******************************************************************************
*
* @description
*    A example of reading NRS1800 Reg by i2c interface.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FI2cPs_example3(void)
{
  u8 Status=FMSH_SUCCESS;
  
  FI2cPs_T *pI2c0_dev = &gI2c0_dev;
  FIicPs_Instance_T *pI2c0_Instance=&gI2c0_Instance;
  FIicPs_Param_T I2c0_Param;
     
  Status = FGicPs_Connect(&IntcInstance, I2C0_INT_ID,
                   (FMSH_InterruptHandler)iic0_interrupt_handler, 0);
  if (Status != GIC_SUCCESS)
  {
    TRACE_OUT(DEBUG_OUT, "Failed to generate I2C0 int.\r\n");
    return Status;
  }
  FGicPs_Enable(&IntcInstance, I2C0_INT_ID);
     
  /* Initial I2C0 */
  FI2c0Ps_DeviceInit(pI2c0_dev,pI2c0_Instance,&I2c0_Param);
  FI2cPs_MasterInit(pI2c0_dev);    //In default config the device as master
  
  FI2cPs_enable(pI2c0_dev);
  
  FI2cPs_ReadNRS1800Reg(pI2c0_dev,0x55<<2,0xaa<<2);
  
  return Status;
}