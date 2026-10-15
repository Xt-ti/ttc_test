/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_can_example.c
*
* This file contains a example of can.
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   wfb  11/23/2018  First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "fmsh_can_lib.h" 
#include "fmsh_gic.h"
#include "fmsh_slcr.h"  

/************************** Constant Definitions *****************************/

#define CAN_INPUT_CLOCK_FREQUENCY 5000000

#define CAN_BUAD_1MHZ 1000000
#define CAN_BUAD_500KHZ 500000
#define CAN_BUAD_250KHZ 250000
#define CAN_BUAD_100KHZ 100000
#define CAN_BUAD_50KHZ 50000
#define CAN_BUAD_25KHZ 25000

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

FCanPs_T g_CAN0, g_CAN1;

u8 g_can_recv_intr_flag;
u8 g_can1rbuf[50];

/****************************************************************************/
/**
*
* This function is used to register interrupt
*
* @param    dev is a pointer to the instance of device.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
static u8 can_setHanlder(FCanPs_T* dev, u32 id, FMSH_InterruptHandler hanlder)
{
    FGicPs_Connect(&IntcInstance, id, hanlder,  dev);
    FGicPs_Enable(&IntcInstance, id); 

    return 0;
}

/****************************************************************************/
/**
*
* This function is used to handle g_CAN1 intrrrupt
*
* @param  None.
*  
* @return   None.
*
* @note     None.
*
****************************************************************************/
static void CAN1_interrupt_hanlder (void *InstancePtr)
{
    u8 reg;

    g_can_recv_intr_flag = 1;
            
    reg = FCanPs_getInterrupt(&g_CAN1);//intr sts reg is read only,all bits will be reset except intr recv bit

    if(FMSH_BIT_GET(reg, CAN_IR_RI) == 1)  //recv buf full
    { 
        FCanPs_frameReceive(&g_CAN1, g_can1rbuf);
        FCanPs_releaseReceiveBuffer(&g_CAN1);
    }   
}

/****************************************************************************/
/**
*
* This function communicte messages between &g_CAN0 & &g_CAN1.
* &g_CAN0 & &g_CAN1 inner loopback by set SLCR register
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
****************************************************************************/
u8 FCanPs_example(void)
{
        u32 i;
        u8 ret=FMSH_SUCCESS;
        u32 tr_val[11];
        u8 buf[8];
        u32 timeout = 0x10000;
        
        for(i = 0; i < 8; i++)
                buf[i] = 0x55 + i;
            
        FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x008, 0xDF0D767BU);//unlock     
        //FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x768, 0x00001621U);// MIO[26] g_CAN0 RX         
        //FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x76C, 0x00001620U);// MIO[27] g_CAN0 TX    
        FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x004, 0xDF0D767BU);//lock    
            
        FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x008, 0xDF0D767BU);//unlock    
        FMSH_WriteReg(FPS_SLCR_BASEADDR,0x304, 0x00000003U);
        FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x804, 0x4);//loop back  
        //FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x760, 0x00001620U);// MIO[24] g_CAN1 TX         
        //FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x764, 0x00001621U);// MIO[25] g_CAN1 RX 
        FMSH_WriteReg(FPS_SLCR_BASEADDR, 0x004, 0xDF0D767BU);//lock              
    
        FCanPs_Config* Config=NULL;
        Config= FCanPs_LookupConfig(FPAR_CANPS_0_DEVICE_ID);
        if(Config==NULL)
            return FMSH_FAILURE;
        ret=FCanPs_init(&g_CAN0, Config);
        if(ret!=FMSH_SUCCESS)
            return FMSH_FAILURE;
        
        FCanPs_setBaudRate(&g_CAN0, CAN_BUAD_1MHZ);
        FCanPs_setStdACR(&g_CAN0, 0x55, 1, 0x66, 0x66, single_filter);
        FCanPs_setAMR(&g_CAN0, 0xff, 0xff, 0xff, 0xff); 

        Config= FCanPs_LookupConfig(FPAR_CANPS_1_DEVICE_ID);
        if(Config==NULL)
          return FMSH_FAILURE;
        ret=FCanPs_init(&g_CAN1, Config);
        if(ret!=FMSH_SUCCESS)
          return FMSH_FAILURE;

        FCanPs_setBaudRate(&g_CAN1, CAN_BUAD_1MHZ);
        FCanPs_setStdACR(&g_CAN1, 0x55, 1, 0x66, 0x66, single_filter);
        FCanPs_setAMR(&g_CAN1, 0xff, 0xff, 0xff, 0xff);    
        
        can_setHanlder(&g_CAN1, CAN1_INT_ID, (FMSH_InterruptHandler)CAN1_interrupt_hanlder);
        
        //FSlcrPS_setCanLoop(1);
        FCanPs_setReceiveInterrupt(&g_CAN1, CAN_set);
  
        g_can_recv_intr_flag = 0;
        FCanPs_standardFrameTransmit(&g_CAN0, 0x55, buf, 8, data_frame);
        FCanPs_transmissionRequest(&g_CAN0);
        timeout = 0x10000;
        while((FCanPs_getTransmissionCompleteStatus(&g_CAN0) == 0)&&(timeout--));
        if(timeout == 0)
                return FMSH_FAILURE;

        if(g_can_recv_intr_flag == 1)
        {
                g_can_recv_intr_flag = 0;
                
                for(i = 0; i < 11; i++)
                {
                        tr_val[i] = FMSH_ReadReg(FPS_CAN0_BASEADDR, 0x180 + 4 * i);
                }
                for(i = 0; i < 11; i++)
                { 
                    if(tr_val[i] != g_can1rbuf[i])
                        return FMSH_FAILURE;
                }
        }
        else
                return FMSH_FAILURE;
        
        FSlcrPS_setCanLoop(0);        
        
        return FMSH_SUCCESS;
}