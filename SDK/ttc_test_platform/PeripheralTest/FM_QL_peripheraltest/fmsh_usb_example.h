/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/******************************************************************************
*
* @file  fmsh_usb_example.h
*
* This file contains public constant & function define of usb driver.
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   lq  08/22/2019  First Release
*</pre>
******************************************************************************/

#ifndef _FMSH_USB_EXAMPLE_H_
#define _FMSH_USB_EXAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif
  
/***************************** Include Files *********************************/
#include "fmsh_common.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
u8 FUsbDfuPs_example(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* end of protection macro */
