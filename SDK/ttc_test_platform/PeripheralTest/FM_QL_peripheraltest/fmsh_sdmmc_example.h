/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/******************************************************************************
*
* @file  fmsh_sdmmc_example.h
*
* This file contains
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   yl  12/20/2018  First Release
*</pre>
******************************************************************************/
#ifndef _FMSH_SDMMC_EXAMPLE_H_
#define _FMSH_SDMMC_EXAMPLE_H_

/***************************** Include Files *********************************/

#include "fmsh_sdmmc_lib.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void FSdmmcPs_IRQ (void *InstancePtr);
s32 FSdmmcPs_example(void);

#endif /* _FMSH_SDMMC_EXAMPLE_H_ */

