/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_gpio_example.c
*
* This file contains a example of gpio.
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
#include "fmsh_gpio_public.h"
#include "fmsh_ps_parameters.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Local Function ***********************************/

/************************** Extern Function **********************************/

#define GPIO_DeviceID FPAR_GPIOPS_0_DEVICE_ID

/******************************************************************************
*
* @description
*    A minimal test of gpio, configure GPIO A as output and drive.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FGpioPs_output_example()
{
    FGpioPs_T gpioDevA;
    FGpioPs_init(&gpioDevA, FGpioPs_LookupConfig(GPIO_DeviceID));

    FGpioPs_setDirection(&gpioDevA, 0xFFFFFFFF);    /*set gpio direction all output */
    FGpioPs_writeData(&gpioDevA, 0x00000000);       /*output low level to gpio pins*/
    delay_ms(1000);
    FGpioPs_writeData(&gpioDevA, 0xFFFFFFFF);       /*output high level to gpio pins*/

    return 0;
}

/******************************************************************************
*
* @description
*    A minimal test of gpio, configure GPIO A as input and get ext data.
*
* @param    None.
*
* @return   0 if successful, otherwise 1.
*
* @note     None.
*
******************************************************************************/
u8 FGpioPs_input_example()
{
    FGpioPs_T gpioDevA;

    u32 readback;

    FGpioPs_init(&gpioDevA, FGpioPs_LookupConfig(GPIO_DeviceID));
    FGpioPs_setDirection(&gpioDevA, 0x0);       /*set gpio direction all input */
    delay_ms(10);
    readback = FGpioPs_getExtPort(&gpioDevA);
    printf("input data is : %x\r\n",readback);

    return 0;
}