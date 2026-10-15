/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/******************************************************************************
*
* @file  fmsh_dmac_example.h
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
#ifndef _FMSH_DMAC_EXAMPLE_H_
#define _FMSH_DMAC_EXAMPLE_H_

/***************************** Include Files *********************************/

#include "fmsh_dmac_lib.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

// define globle list
extern FMSH_listHead g_DMA_list;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

void FDmaPs_IRQ (void *InstancePtr);
s32 FDmaPs_example(void);


#endif /* #ifndef _FMSH_DMAC_EXAMPLE_H_ */

