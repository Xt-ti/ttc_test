/******************************************************************************
*
* Copyright (C) 2018 - 2028 FMSH, Inc.  All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  fmsh_gmac_example_simple.c
*
* a simple example : rcv then send
*
* @note		None.
*
* MODIFICATION HISTORY:
*
*<pre>
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------
* 0.01   Hansen Yang  12/24/2018  First Release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdlib.h>
#include "fmsh_gic.h"
#include "fmsh_gmac_lib.h"
#include "fmsh_mdio.h"
#include "fmsh_common.h"
#include "fmsh_gpio_public.h"

/************************** Constant Definitions *****************************/
#define ENET_MAC_ADDRESS 0x5a,0x01,0x02,0x03,0x04,0x05
#define GMAC_TESTFRAME_HEAD 0x5A,0x01,0x02,0x03,0x04,0x05,0x5A,0x06,0x07,0x08,0x09,0x0A,0x88,0x00
#define GMAC_TEST_FRAME_DATA_LENGTH 300 /* 0-1500 */
#define GMAC_DEBUG_OUT 0

#define GMAC_VERIFY_ID              FPAR_GMACPS_0_DEVICE_ID

#define ENET_MAC_ADDRESS 0x5a,0x01,0x02,0x03,0x04,0x05

#define GMAC_RDES_NUM 			5		/* number of Rx DES */
											/* for normal test 9212-Byte-length frame, BUFFERSIZE * UNITSIZE * 2 >= 9212 */
#define GMAC_TDES_NUM 			5		/* number of Tx DES */

#define GMAC_PACKET_BUFFER_SIZE       1000    /* RxPacketBuffer size */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
extern u32 mvl88e1116r_pcs_lpbk(FGmacPs_Instance_T *pGmac, u8 enable,u8 speed);
u8 FGmacPs_IrqHandler(FGmacPs_Instance_T *p_dev);
/************************** Variable Definitions *****************************/
static FGmacPs_Instance_T 	    s_GMAC_Instance={
    .mac_address={ENET_MAC_ADDRESS},
    .csr_clk=5,
};
static FGmacPs_Instance_T       *s_GMAC_instance_p=&s_GMAC_Instance;
static FGmacPs_LinkStatus_T	    s_GMAC_LinkStatus;
static FGmacPs_PhyConfig_T	    s_GMAC_PhyCfg={
    .phy_device=PHY_88E1116R,
    .auto_detect_ad_en=1,
    .mdio_address=0,
    .auto_nag_en=1,
};

static u8 s_tx_done_flag=0;
static u8 s_rcv_flag=0;
static u32 s_status_reg=0;
static u8 s_rcv_frame_cnt=0;
/* place in UnCached section */
FGmacPs_RxDescriptor_T GMAC0_RxDs[GMAC_RDES_NUM]@ "GMAC_DES";
FGmacPs_TxDescriptor_T GMAC0_TxDs[GMAC_TDES_NUM]@ "GMAC_DES";
/************************** Function *****************************************/
/*  interrupt Func */
void gmac_interrupt_handler(void)
{
    FGmacPs_IrqHandler(s_GMAC_instance_p);
}

u8 FGmacPs_IrqHandler(FGmacPs_Instance_T *pGmac)
{
	u8 rtval;
	FGmacPs_DmaPortMap_T *pDma;
	FMSH_callback   userCallback;
	u32 reg_val_32b=0;
	u32 callbackArg;
	FGmacPs_DmaIrq_T clearIrqMask;

	/*------variables init------------------------------*/
	pDma = (FGmacPs_DmaPortMap_T *)((u32)pGmac->base_address + GMAC_DMA_OFFSET);

	userCallback = NULL;
	callbackArg = 0;
	clearIrqMask = gdma_irq_none;
	rtval = GMAC_RETURN_CODE_OK;

	/* what caused the interrupt?*/
	reg_val_32b = FMSH_IN32_32(pDma->GDMA_SR);
    s_status_reg=s_status_reg|reg_val_32b;
	if((reg_val_32b & gdma_irq_aie) != 0)
	{
		userCallback = pGmac->listener;
		if((reg_val_32b&gdma_irq_rx_wd_timeout) != 0)   /* Receive watchdog timeout */
		{
			callbackArg  = gdma_irq_rx_wd_timeout;
			clearIrqMask = gdma_irq_rx_wd_timeout;
		}
		else if((reg_val_32b&gdma_irq_tx_jabber_timeout) != 0)    /* Tansmit jabber timeout */
		{
			callbackArg  = gdma_irq_tx_jabber_timeout;
			clearIrqMask = gdma_irq_tx_jabber_timeout;
		}
		else if((reg_val_32b&gdma_irq_rx_overflow) != 0)   /* Receive FIFO overflow */
		{
			callbackArg  = gdma_irq_rx_overflow;
			clearIrqMask = gdma_irq_rx_overflow;
		}
		else if((reg_val_32b&gdma_irq_rx_unbuffer) != 0)   /* Receive buffer unavailable */
		{
			callbackArg  = gdma_irq_rx_unbuffer;
			clearIrqMask = gdma_irq_rx_unbuffer;
		}
		else if((reg_val_32b&gdma_irq_rx_stop) != 0)   /* Receive Process stopped */
		{
			callbackArg  = gdma_irq_rx_stop;
			clearIrqMask = gdma_irq_rx_stop;
		}
		else if((reg_val_32b&gdma_irq_tx_stop) != 0)   /* Transmit Process stopped */
		{
			callbackArg  = gdma_irq_tx_stop;
			clearIrqMask = gdma_irq_tx_stop;
		}
		else if((reg_val_32b&gdma_irq_early_tx) != 0)   /* Early Transmit interrupt */
		{
			callbackArg  = gdma_irq_early_tx;
			clearIrqMask = gdma_irq_early_tx;
		}
		else if((reg_val_32b&gdma_irq_tx_underflow) != 0)   /* Transmit underflow */
		{
			callbackArg  = gdma_irq_tx_underflow;
			clearIrqMask = gdma_irq_tx_underflow;
		}
		else if((reg_val_32b&gdma_irq_fatal_bus) != 0)   /* Fatal bus error */
		{
			callbackArg  = gdma_irq_fatal_bus;
			clearIrqMask = gdma_irq_fatal_bus;
		}
	}
	else if((reg_val_32b & gdma_irq_nie) != 0)
	{
		if((reg_val_32b&gdma_irq_rx) != 0)   /* receive interrupt */
		{
			userCallback = pGmac->rxCallback;
			callbackArg  = gdma_irq_rx;
			clearIrqMask = gdma_irq_rx;
		}
		else if((reg_val_32b&gdma_irq_early_rx) != 0)   /* Early receive interrupt */
		{
			userCallback = pGmac->listener;
			callbackArg  = gdma_irq_early_rx;
			clearIrqMask = gdma_irq_early_rx;
		}
		else if((reg_val_32b&gdma_irq_tx) != 0)   /* Transmit interrupt */
		{
			userCallback = pGmac->txCallback;
			callbackArg  = gdma_irq_tx;
			clearIrqMask = gdma_irq_tx;
		}
		else if((reg_val_32b&gdma_irq_tx_unbuffer) != 0)   /* Transmit buffer unavailable */
		{
			userCallback = pGmac->listener;
			callbackArg  = gdma_irq_tx_unbuffer;
			clearIrqMask = gdma_irq_tx_unbuffer;
		}
	}
	else if((reg_val_32b & gdma_irq_gli) != 0) 				/* GMAC Line Interface Interrupt */
	{
		userCallback = pGmac->listener;
		callbackArg  = gdma_irq_gli;
		clearIrqMask = gdma_irq_none;							/* GLI has been cleared after listener function */
	}
	else
	{
		TRACE_OUT(GMAC_DEBUG_OUT, "\r\n");
		TRACE_OUT(GMAC_DEBUG_OUT, "!!! Unknown Interrupt !!!\r\n");
		rtval = GMAC_RETURN_CODE_ERR;
		return rtval;
	}

	/* -----call the user listener function------------- */
	if(userCallback != NULL)
		userCallback(pGmac,callbackArg);

	/*--------clear the serviced interrupt-------------- */
	if(clearIrqMask != 0)
	{
		FGmac_Ps_ClearIrq(pGmac,clearIrqMask);
		reg_val_32b = FMSH_IN32_32(pDma->GDMA_SR);
		/* clear aie summary */
		if((reg_val_32b&gdma_irq_all_ai)==0)
		{
			FMSH_OUT32_32(gdma_irq_aie,pDma->GDMA_SR);
		}
		/* clear nie summary */
		if((reg_val_32b&gdma_irq_all_ni)==0)
		{
			FMSH_OUT32_32(gdma_irq_nie,pDma->GDMA_SR);
		}
	}
	reg_val_32b = FMSH_IN32_32(pDma->GDMA_SR);
	return rtval;
}

void FGmacPs_GmacListener(FGmacPs_Instance_T *pGmac,int32_t ecode)
{
	FGmacPs_MacPortMap_T *pGmacPortMap;
	pGmacPortMap = pGmac->base_address;
    FGmacPs_DmaPortMap_T *pDmaPortMap;
    pDmaPortMap = (FGmacPs_DmaPortMap_T*)((u32)pGmac->base_address + GMAC_DMA_OFFSET);
	u32 reg;
	switch(ecode)
	{
	case gdma_irq_tx_stop:
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:Tx process stopped \r\n");
		break;
	case gdma_irq_tx_unbuffer:
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:Tx Buffer Unavailable \r\n");
		break;
	case gdma_irq_tx_jabber_timeout:
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:Tx jabber timeout \r\n");
		break;
	case gdma_irq_rx_overflow:
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:Rx FIFO overflow \r\n");
		break;
	case gdma_irq_tx_underflow:
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:Tx underflow \r\n");
		break;
	case gdma_irq_rx_unbuffer:
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:Rx buffer unavailable \r\n");
		TRACE_OUT(GMAC_DEBUG_OUT, "Reset this Rx DES  \r\n");
		reg=FMSH_IN32_32(pDmaPortMap->GDMA_CRXDES);
		FGmac_Ps_ResetCurRxDES(pGmac,(FGmacPs_RxDescriptor_T *)reg);
		break;
	case gdma_irq_rx_stop:
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:Rx process stopped \r\n");
		break;
	case gdma_irq_rx_wd_timeout:
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:Rx watchdog timeout \r\n");
		break;
	case gdma_irq_early_tx:
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:Early Tx interrupt \r\n");
		break;
	case gdma_irq_fatal_bus:
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:Fatal bus error \r\n");
		break;
	case gdma_irq_early_rx:
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:Early Rx interrupt \r\n");
		break;
	case gdma_irq_gli:  /* not finished */
		TRACE_OUT(GMAC_DEBUG_OUT, "> Irq:GMAC Line Interface Interrupt \r\n");
		reg=FMSH_IN32_32(pGmacPortMap->GMAC_ISR);
		if((reg&GMAC_ISR_RGSMIIIS) != 0)
		{
			TRACE_OUT(GMAC_DEBUG_OUT, "RGMII or SMII Interrupt, Link status change \r\n");
			FGmac_Ps_GetLinkStatus(pGmac);  /* GLI will be cleared when read these bits */
		}
		break;
	default:
		TRACE_OUT(GMAC_DEBUG_OUT, "> unexpected argument: 0x%x \r\n",ecode);
		break;
	}
}

void FGmacPs_GmacRxCallback(FGmacPs_Instance_T *pGmac,int32_t ecode)
{
	uint32_t frmSize;
	u8      *pBuffer;
	u8 j =0;
	u8 isBroadcast = 0;
	u32 poll_result;
	pBuffer  = pGmac->pFrmBuffer;

	TRACE_OUT(GMAC_DEBUG_OUT, "> Irq: Recevie interrupt \r\n");
	poll_result = FGmac_Ps_RcvPoll(pGmac,&frmSize);
	pGmac->RxFrameSize = frmSize;
	s_rcv_flag=1;
	s_rcv_frame_cnt++;

	if(poll_result == GMAC_RETURN_CODE_OK)
	{
		TRACE_OUT(GMAC_DEBUG_OUT, "A Frame is recevied successfully,size is %d !\r\n",frmSize);
//#if (1==GMAC_DEBUG_OUT)
//        for(u32 tmp_i=0;tmp_i<frmSize;tmp_i++)
//            TRACE_OUT(GMAC_DEBUG_OUT, "%x ",pBuffer[tmp_i]);
//#endif
	}
	else if(poll_result == GMAC_RETURN_CODE_RX_NULL)
	{
		TRACE_OUT(GMAC_DEBUG_OUT, "Rx Frame Error: No data..\r\n");
		return;
	}
	else if(poll_result == GMAC_RETURN_CODE_SIZE_TOO_SMALL)
	{
		TRACE_OUT(GMAC_DEBUG_OUT, "Rx Frame Error: Buffer size is too small..\r\n");
		return;
	}

	/*** Detect which cast: unicast,multicast,broadcast ***/
	while(pBuffer[j] == 0xFF)
	{
		j++;
		if(j== 6)
		{
			isBroadcast = 1;
			break;
		}
	}
	if(isBroadcast)
	{
		TRACE_OUT(GMAC_DEBUG_OUT, "Received a broadcast frame.\r\n");
	}
	else if((pBuffer[0]&0x01) == 0x01)
	{
		TRACE_OUT(GMAC_DEBUG_OUT, "Received a multicast frame.\r\n");
	}
	else {TRACE_OUT(GMAC_DEBUG_OUT, "Received a unicast frame.\r\n");}
}

void FGmacPs_GmacTxCallback(FGmacPs_Instance_T *pGmac,int32_t ecode)
{
	TRACE_OUT(GMAC_DEBUG_OUT, "> Irq: Transmit interrupt \r\n");
	TRACE_OUT(GMAC_DEBUG_OUT, "Sending frame is done!\r\n");
	s_tx_done_flag = 1;
}

void FGmacPs_88e1116rHwRst()
{
    /* 88e1116r HW reset */
    FGpioPs_T gpioDevB;
    FGpioPs_Config *pGpioCfg;
    pGpioCfg = FGpioPs_LookupConfig(FPAR_GPIOPS_1_DEVICE_ID);
    FGpioPs_init(&gpioDevB, pGpioCfg);
    FGpioPs_setDirection(&gpioDevB, 0xffffffff);  //1-out  0-in
    FGpioPs_writeData(&gpioDevB, 0xffffffff);
    FGpioPs_writeData(&gpioDevB, 0);
    delay_ms(200);
    FGpioPs_writeData(&gpioDevB, 0xffffffff);
}

/****************************************************************************/
/**
*
* gmac0 simple example, 88e1116r PCS loopback.
* Tx send a frame, phy echo, Rx rcv and check the frame.
*
* @param    None.
*
* @return   success: 0, fail: 1
*
****************************************************************************/
u8 FGmacPs_PhyPcsLoopBackExample(void)
{
    FGmacPs_88e1116rHwRst();

    u8* GMAC0_PacketBuffer;
    u8* GMAC0_RxBuffer;
    u8* GMAC0_TxBuffer;

    /* malloc mem */
    GMAC0_PacketBuffer=(u8 *)malloc(GMAC_PACKET_BUFFER_SIZE);
    if(GMAC0_PacketBuffer==NULL){
        TRACE_OUT(GMAC_DEBUG_OUT,"gmac lib malloc fail\r\n");
        return GMAC_RETURN_CODE_ERR;
    }

    GMAC0_RxBuffer=(u8 *)malloc(2*GMAC_RDES_NUM*GMAC_RBUFFER_UNIT_SIZE);
    if(GMAC0_RxBuffer==NULL){
        TRACE_OUT(GMAC_DEBUG_OUT,"gmac lib malloc fail\r\n");
        return GMAC_RETURN_CODE_ERR;
    }

    GMAC0_TxBuffer=(u8*)malloc(2*GMAC_TDES_NUM*GMAC_TBUFFER_UNIT_SIZE);
    if(GMAC0_TxBuffer==NULL){
        TRACE_OUT(GMAC_DEBUG_OUT,"gmac lib malloc fail\r\n");
        return GMAC_RETURN_CODE_ERR;
    }

    /* setup gmac */
    FGmacPs_Config_T *pGmacCfg;
    pGmacCfg = FGmacPs_LookupConfig(FPAR_GMACPS_0_DEVICE_ID);
    FGmac_Ps_StructInit(s_GMAC_instance_p,
                        &s_GMAC_LinkStatus,
                        pGmacCfg,
                        &s_GMAC_PhyCfg,
                        GMAC_RDES_NUM, GMAC_TDES_NUM,
                        GMAC_RBUFFER_UNIT_SIZE, GMAC_TBUFFER_UNIT_SIZE,
                        GMAC0_PacketBuffer, GMAC_PACKET_BUFFER_SIZE);
    FGmac_Ps_DeviceReset(s_GMAC_instance_p);
    FGmac_Ps_DmaInit(s_GMAC_instance_p,
                     GMAC0_RxDs,
                     GMAC0_RxBuffer,
                     GMAC0_TxDs,
                     GMAC0_TxBuffer);
	FGmac_Ps_MacInit(s_GMAC_instance_p);
	FGmac_Ps_SetListener(s_GMAC_instance_p,(FMSH_callback)FGmacPs_GmacListener);
	FGmac_Ps_SetTxCallback(s_GMAC_instance_p,(FMSH_callback)FGmacPs_GmacTxCallback);
	FGmac_Ps_SetRxCallback(s_GMAC_instance_p,(FMSH_callback)FGmacPs_GmacRxCallback);

    /* gic */
    FGicPs_SetupInterruptSystem(&IntcInstance);
    FGicPs_Connect(&IntcInstance,GMAC0_INT_ID,(FMSH_InterruptHandler)gmac_interrupt_handler,0);
    FMSH_ExceptionRegisterHandler(FMSH_EXCEPTION_ID_FIQ_INT,(FMSH_ExceptionHandler)FGicPs_InterruptHandler_FIQ,&IntcInstance);
    FGicPs_Enable(&IntcInstance,GMAC0_INT_ID);

    /* enable phy PCS loopback */
    FGmacPs_Speed LinkSpeed=FPAR_GMACPS_0_SPEED;
    if(LinkSpeed==speed_10)
    {
        mvl88e1116r_pcs_lpbk(s_GMAC_instance_p,1,0);
    }else if(LinkSpeed==speed_100)
    {
        mvl88e1116r_pcs_lpbk(s_GMAC_instance_p,1,1);
    }else if(LinkSpeed==speed_1000)
    {
        mvl88e1116r_pcs_lpbk(s_GMAC_instance_p,1,2);
    }

    /* setup frame */
    u8 mac_frame[GMAC_TEST_FRAME_DATA_LENGTH+14]={GMAC_TESTFRAME_HEAD};
    for(u32 i=0;i<GMAC_TEST_FRAME_DATA_LENGTH;i++)
    {
        mac_frame[14+i]=(u8)i;
    }
    /* send frame */
    FGmac_Ps_Send(s_GMAC_instance_p,mac_frame,GMAC_TEST_FRAME_DATA_LENGTH+14,0,0);
    /* wait for tx interrupt */
    u32 timeout=0;
    while(0==s_tx_done_flag)
    {
        if(timeout>100)
        {
            TRACE_OUT(GMAC_DEBUG_OUT,"GMAC TX interrupt time out.\n");
            return -1;
        }
        timeout++;
        delay_ms(1);
    }
    s_tx_done_flag=0;

    /* wait for rx interrupt */
    timeout=0;
    while(0==s_rcv_flag)
    {
        if(timeout>1000)
        {
            TRACE_OUT(GMAC_DEBUG_OUT,"GMAC RX interrupt time out.\n");
            return -1;
        }
        timeout++;
        delay_ms(1);
    }
    s_rcv_flag=0;
    /* check data */
	u8 check_result=0;
    for (u32 i=0;i<GMAC_TEST_FRAME_DATA_LENGTH+14;i++){
        if(*((u8*)(s_GMAC_instance_p->pFrmBuffer+i))!=mac_frame[i]){
            check_result=1;
            break;
        }
    }

    TRACE_OUT(GMAC_DEBUG_OUT,"gmac check result:%d\r\n",check_result);
    return check_result;
}
