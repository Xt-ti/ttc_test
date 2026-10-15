/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (JFM_QL_PS) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bsp)
 */

#include <stdio.h>
#include <math.h>
#include "platform.h"
#include "fmsh_common.h"
#include "ps_init.h"
#include "fmsh_print.h"
#include "fmsh_ttc_public.h"
#include "fmsh_gic.h"

#define   TTC_INTR_NUM_TEST     0

FTtcPs_T ttc_dev;
volatile u8 g_ttc0_flag = 0;
u32 gTtcIsrCnt = 0;
u32 printFlag  = 0;
u64 gtCurPrev = 0;
u64 gtTime = 0;
u64 gtCur  = 0;
u32 gtTime32 = 0;

void timer1_handler(FTtcPs_T *DevPtr)
{
    #if TTC_INTR_NUM_TEST
    FTtcPs_ClearTimerNInterrupt(DevPtr, 1);
    // FTtcPs_setTimerEnble(DevPtr, timer1, FMSH_clear);
    g_ttc0_flag = 1;
    gTtcIsrCnt++;
    
    //tCur = get_current_time();
    //gtTime = tCur - gtCurPrev;
    //gtTime32 = (u32)gtTime;
    //gtCurPrev = tCur;
    #else
    u64 tCur;
    
    u32 auctemp[20] = {1,2,3};
    
    FTtcPs_ClearTimerNInterrupt(DevPtr, 1);
    // FTtcPs_setTimerEnble(DevPtr, timer1, FMSH_clear);
    g_ttc0_flag = 1;
    gTtcIsrCnt++;
    tCur = get_current_time();
    gtTime = tCur - gtCurPrev;
    gtTime32 = (u32)gtTime;
    gtCurPrev = tCur;
    
    auctemp[0]++;
    //fmsh_print("aa:%x", 12);
    //fmsh_print("aabb\n\r");
    
    #endif
}

int main()
{
    init_platform();

    fmsh_print("Hello World\n\r");

    global_timer_enable();

    FTtcPs_Config* cfg=NULL;
    cfg=FTtcPs_LookupConfig(FPAR_TTCPS_0_DEVICE_ID);
    FTtcPs_init(&ttc_dev, cfg);
    cfg=NULL;
    FGicPs_SetupInterruptSystem(&IntcInstance);
    FGicPs_Connect(&IntcInstance, TIMER0_1_INT_ID, (FMSH_InterruptHandler)timer1_handler, &ttc_dev);
    FMSH_ExceptionRegisterHandler(FMSH_EXCEPTION_ID_IRQ_INT,
          (FMSH_ExceptionHandler)FGicPs_InterruptHandler_IRQ, &IntcInstance);
    FGicPs_Enable(&IntcInstance, TIMER0_1_INT_ID);

    FTtcPs_setTimerEnble(&ttc_dev, timer1, FMSH_clear);
    FTtcPs_setTimerMode(&ttc_dev, timer1, user_DefinedCount_mode);
    FTtcPs_setTimerInterruptMask(&ttc_dev, timer1, FMSH_clear);
    FTtcPs_setTIMERPWM(&ttc_dev, timer1, FMSH_clear);
    //FTtcPs_TimerNLoadCount(&ttc_dev, timer1, 83333332);  // 1s周期值
    FTtcPs_TimerNLoadCount(&ttc_dev, timer1, 10 * TTC0_0_FREQ); 
    //FTtcPs_TimerNLoadCount(&ttc_dev, timer1, 380);  // 1s周期值

    g_ttc0_flag = 0;
    FTtcPs_setTimerEnble(&ttc_dev, timer1, FMSH_set);
    
    #if TTC_INTR_NUM_TEST
    while(1) {
        //while(0 == g_ttc0_flag);
        //g_ttc0_flag = 0;
        
        gtCur    = get_current_time();
        //gtTime   = gtCur - gtCurPrev;
        //gtTime32 = (u32)gtTime;
        
        if ((gTtcIsrCnt - printFlag) > 1000000)
        {
            printFlag = gTtcIsrCnt;
            fmsh_print("Num:%d cycles, %.2f s\n\r", gTtcIsrCnt, (float)gtCur/GTC_CLK_FREQ);
        }
        //gtCurPrev = gtCur;
        
        //fmsh_print("Time:%d cycles, %.2f s\n\r", gtTime32, (float)gtTime32/GTC_CLK_FREQ);
    }
    #else
    while(1) 
    {
        while(0 == g_ttc0_flag);
        g_ttc0_flag = 0;
        fmsh_print("Time:%d cycles, %.2f s\n\r", gtTime32, (float)gtTime32/GTC_CLK_FREQ);
    }
    #endif

    fmsh_print("Test finished!\n\r");

    return 0;
}
