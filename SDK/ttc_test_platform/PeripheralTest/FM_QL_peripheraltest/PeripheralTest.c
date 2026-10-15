#include "fmsh_common.h"
#include "ps_init.h"
#include "fmsh_gic.h"
#include "cache.h"

#if defined(CANPS_0_DEVICE_ID) && defined(CANPS_1_DEVICE_ID)
#include "fmsh_can_example.h"
#endif

#ifdef FPAR_DMAPS_DEVICE_ID
#include "fmsh_dmac_example.h"
#endif

#ifdef GPIOPS_0_DEVICE_ID
#include "fmsh_gpio_example.h"
#endif

#ifdef ENETPS_0_DEVICE_ID
#include "fmsh_gmac_example.h"
#endif

#ifdef I2CPS_0_DEVICE_ID
#include "fmsh_i2c_example.h"
#endif

#ifdef NANDPS_0_DEVICE_ID
#include "fmsh_nfc_example.h"
#endif

#ifdef QSPIPS_0_DEVICE_ID
#include "fmsh_qspi_example.h"
#endif

#ifdef SDMMCPS_0_DEVICE_ID
#include "fmsh_sdmmc_example.h"
#endif

#ifdef SMCPS_0_DEVICE_ID
#include "fmsh_smc_example.h"
#endif

#if defined(SPIPS_0_DEVICE_ID) && defined(SPIPS_1_DEVICE_ID)
#include "fmsh_spi_example.h"
#endif

#ifdef TIMERPS_0_DEVICE_ID
#include "fmsh_ttc_example.h"
#endif

#ifdef UARTPS_0_DEVICE_ID
#include "fmsh_uart_example.h"
#endif

#ifdef USBPS_0_DEVICE_ID
#include "fmsh_usb_example.h"
#endif

u8 main()
{
    u8 Status=FMSH_SUCCESS;
    
#if PSOC_CACHE_ENABLE
    dcache_enable();
#endif
    
    Status=ps_init();
    if(Status!=PS_INIT_SUCCESS)
    {
      TRACE_OUT(DEBUG_OUT, " PS Initial Failed!\r\n");
      return Status;
    }
    
	Status =  FGicPs_SelfTest(&IntcInstance);
	if(Status!=GIC_SUCCESS)
		TRACE_OUT(DEBUG_OUT, " GIC Setup Failed!\r\n");
	else
		TRACE_OUT(DEBUG_OUT, " GIC Setup pass!\r\n");
        
#ifdef GPIOPS_0_DEVICE_ID
    FGpioPs_output_example();
    FGpioPs_input_example();
#endif
    
#ifdef UARTPS_0_DEVICE_ID
    Status = FUartPs_example();
    if(Status == 0)
        TRACE_OUT(DEBUG_OUT,"UART example test pass!\n");
    else
        TRACE_OUT(DEBUG_OUT,"UART example test failed,please check!\n");
#endif

#ifdef FPAR_DMAPS_DEVICE_ID
    Status = FDmaPs_example();
    if(Status == 0)
        TRACE_OUT(DEBUG_OUT,"DMA example test pass!\n");
    else
        TRACE_OUT(DEBUG_OUT,"DMA example test failed,please check!\n");
#endif

#ifdef ENETPS_0_DEVICE_ID
	Status = FGmacPs_PhyPcsLoopBackExample();
    if(Status == 0)
        TRACE_OUT(DEBUG_OUT,"GMAC example test pass!\n");
    else
        TRACE_OUT(DEBUG_OUT,"GMAC example test failed,please check!\n");
#endif

#ifdef NANDPS_0_DEVICE_ID
        Status = FNfcPs_example();
        if(Status == 0)
            TRACE_OUT(DEBUG_OUT,"NFC example test pass!\n");
        else
            TRACE_OUT(DEBUG_OUT,"NFC example test failed,please check!\n");
#endif


#ifdef SMCPS_0_DEVICE_ID
	Status = FSmcPs_example();
    if(Status == 0)
        TRACE_OUT(DEBUG_OUT,"SMC example test pass!\n");
    else
        TRACE_OUT(DEBUG_OUT,"SMC example test failed,please check!\n");
#endif

#ifdef SDMMCPS_0_DEVICE_ID
	Status = FSdmmcPs_example();
    if(Status == 0)
        TRACE_OUT(DEBUG_OUT,"SDMMC example test pass!\n");
    else
        TRACE_OUT(DEBUG_OUT,"SDMMC example test failed,please check!\n");
#endif

#if defined(CANPS_0_DEVICE_ID) && defined(CANPS_1_DEVICE_ID)
    Status = FCanPs_example();
    if(Status == 0)
        TRACE_OUT(DEBUG_OUT,"CAN example test pass!\n");
    else
        TRACE_OUT(DEBUG_OUT,"CAN example test failed,please check!\n");
#endif

#if defined(I2CPS_0_DEVICE_ID) && defined(I2CPS_1_DEVICE_ID)
    Status = FI2cPs_example();
    if(Status == 0)
        TRACE_OUT(DEBUG_OUT,"I2C example test pass!\n");
    else
        TRACE_OUT(DEBUG_OUT,"I2C example test failed,please check!\n");
#endif    

#if defined(SPIPS_0_DEVICE_ID) && defined(SPIPS_1_DEVICE_ID)
    Status = FSpiPs_example();
    if(Status == 0)
        TRACE_OUT(DEBUG_OUT,"SPI example test pass!\n");
    else
        TRACE_OUT(DEBUG_OUT,"SPI example test failed,please check!\n");
#endif   
    
#ifdef QSPIPS_0_DEVICE_ID
    Status = FQspiPs_example();
    if(Status == 0)
        TRACE_OUT(DEBUG_OUT,"QSPI example test pass!\n");
    else
        TRACE_OUT(DEBUG_OUT,"QSPI example test failed,please check!\n");
#endif

#ifdef TIMERPS_0_DEVICE_ID
    Status = FTtcPs_example();
    if(Status == 0)
        TRACE_OUT(DEBUG_OUT,"TTC example test pass!\n");
    else
        TRACE_OUT(DEBUG_OUT,"TTC example test failed,please check!\n");
#endif

#ifdef USBPS_0_DEVICE_ID  
    Status = FUsbDfuPs_example();
    if(Status == 0)
        TRACE_OUT(DEBUG_OUT,"USB example test pass!\n");
    else
        TRACE_OUT(DEBUG_OUT,"USB example test failed,please check!\n");
#endif
    
    return 0;
}