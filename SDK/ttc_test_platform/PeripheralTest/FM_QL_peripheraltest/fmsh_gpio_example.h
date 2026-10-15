/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_gpio_example.h
*
*
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   yhs  11/23/2018  First Release
*</pre>
******************************************************************************/

#ifndef _FMSH_GPIO_EXAMPLE_H_
#define _FMSH_GPIO_EXAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif
  
/***************************** Include Files *********************************/
#include "fmsh_common.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Local Function ***********************************/

/************************** Extern Function **********************************/
u8 FGpioPs_output_example();
u8 FGpioPs_input_example();
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */