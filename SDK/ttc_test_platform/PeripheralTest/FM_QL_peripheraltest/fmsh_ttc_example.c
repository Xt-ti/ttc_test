/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_ttc_example.c
*
* This file contains a example of ttc.
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

#include "fmsh_ttc_public.h"  
#include "fmsh_gic.h"  

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

volatile u8 g_ttc0_flag = 0;

void timer1_handler(FTtcPs_T *DevPtr)
{
    FTtcPs_ClearTimerNInterrupt(DevPtr, 1);
    FTtcPs_setTimerEnble(DevPtr, timer1, FMSH_clear);
    g_ttc0_flag = 1;
}

/******************************************************************************
*
* @description
*    A example of ttc, when gtc current value reach 0, trigger interrupt.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FTtcPs_example()
{
    FTtcPs_T ttc_dev;
    FTtcPs_Config* cfg=NULL;
    cfg=FTtcPs_LookupConfig(FPAR_TTCPS_0_DEVICE_ID);
    FTtcPs_init(&ttc_dev, cfg);
    cfg=NULL;
    FGicPs_Connect(&IntcInstance, TIMER0_1_INT_ID, (FMSH_InterruptHandler)timer1_handler, &ttc_dev);
    FGicPs_Enable(&IntcInstance, TIMER0_1_INT_ID);   
    
    FTtcPs_setTimerEnble(&ttc_dev, timer1, FMSH_clear);
    FTtcPs_setTimerMode(&ttc_dev, timer1, user_DefinedCount_mode);
    FTtcPs_setTimerInterruptMask(&ttc_dev, timer1, FMSH_clear); 
    FTtcPs_setTIMERPWM(&ttc_dev, timer1, FMSH_clear); 
    FTtcPs_TimerNLoadCount(&ttc_dev, timer1, 0x100000);
    
    g_ttc0_flag = 0;
    FTtcPs_setTimerEnble(&ttc_dev, timer1, FMSH_set);
    
    while(g_ttc0_flag == 0);
    
    return 0;
}
