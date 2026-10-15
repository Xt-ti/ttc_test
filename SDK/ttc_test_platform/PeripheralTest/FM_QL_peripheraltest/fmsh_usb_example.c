/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/******************************************************************************
*
* @file  fmsh_usb_example.c
*
* This file contains a example of usb.
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   lq  1/26/2021  First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "fmsh_ps_parameters.h"
#include "fmsh_usb_example.h" 
#include "fmsh_gic.h" 
#include "fmsh_usb_lib.h" 

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define DFU_DOWNLOAD_ADDRESS 0xE1FE0000

/************************** Variable Definitions *****************************/
FUsbPs_T g_USB0;
u8 g_bUsbInitFirst = TRUE;
extern void FMSH_EXIT(u32 HandOffAddr,u32 Flag);
/************************** Function Prototypes ******************************/

u8 FUsbDfuPs_DownloadImage(FUsbPs_T *usbDev,u32 DownloadAddress)
{
	if(g_bUsbInitFirst == TRUE)
	{
		FUsbPs_InitOtg(usbDev,USB_FULL);
                FUsbPs_SetDownLoadAddr(DownloadAddress);
		g_bUsbInitFirst = FALSE;
	}
        
	FUsbPs_ClearDownFileInfo();
	FUsbPs_SetOpMode(usbDev,USB_CPU);
	
	do{
          
        }while(FUsbPs_IsUsbOtgSetConfiguration()==FALSE);
                
        
        while(1)
        {
            if(FUsbPs_GetDownLoadComplete()==TRUE)
              break;
            delay_ms(10);
        }
        
        delay_ms(100);
        
        FUsbPs_SoftResetCore(usbDev);
        
        FGicPs_Disable(&IntcInstance, USB0_INT_ID); 
        
        (*(void (*)(void))(DownloadAddress))();

	return 0;
}


/****************************************************************************
*
* @description
*    Used to test UARTs' fuction.
*
* @param none.
*  
* @return none.
*
* @note     none.
*
****************************************************************************/
u8 FUsbDfuPs_example(void)
{
    u8 ret=FMSH_SUCCESS;
    FUsbPs_Config *Config=NULL;
    
    Config= FUsbPs_LookupConfig(FPAR_USBPS_0_DEVICE_ID);
    if(Config==NULL)
      return FMSH_FAILURE;
    
    ret=FUsbPs_init(&g_USB0, Config);
    if(ret!=FMSH_SUCCESS)
      return ret;
    
    ret=FGicPs_Connect(&IntcInstance, USB0_INT_ID, (FMSH_InterruptHandler)FUsbPs_HandleEvent, &g_USB0);
    if(ret!=FMSH_SUCCESS)
      return ret;
    
    FGicPs_Enable(&IntcInstance, USB0_INT_ID); 
    
    FUsbDfuPs_DownloadImage(&g_USB0,DFU_DOWNLOAD_ADDRESS);
    
    return FMSH_SUCCESS;
}
