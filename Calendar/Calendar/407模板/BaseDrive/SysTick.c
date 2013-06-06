/******************************************************************
**	  红龙407开发板（V1.0）
**	  SysTick配置文件
**
**	  论    坛：bbs.openmcu.com
**	  旺    宝：www.openmcu.com
**	  邮    箱：support@openmcu.com
**
**    版    本：V1.0
**	  作    者：FXL
**	  完成日期:	2013.1.4
********************************************************************/
#include "stm32f4xx.h"
#include "SysTick.h"
#include "ucos_ii.h"
//
//__IO uint32_t TimingDelay;
//
///******************************************************************************************
//*函数名称：void SysTick_Init(void)
//*
//*入口参数：无
//*
//*出口参数：无
//*
//*功能说明：SysTick初始化  如果初始化失败，会停入WHILE死循环
//*******************************************************************************************/
//void SysTick_Init(void)
//{
//    if (SysTick_Config(SystemCoreClock / 1000))    //1ms
//    { 
//        while (1); //初始化失败
//    }
//}
//
///******************************************************************************************
//*函数名称：void Delay(__IO uint32_t nTime)
//*
//*入口参数：无
//*
//*出口参数：无
//*
//*功能说明：供外部调用的延时函数
//*******************************************************************************************/
//void Delay(__IO uint32_t nTime)
//{ 
//    TimingDelay = nTime;
//    
//    while(TimingDelay != 0);
//}
//
///******************************************************************************************
//*函数名称：void TimingDelay_Decrement(void)
//*
//*入口参数：无
//*
//*出口参数：无
//*
//*功能说明：SysTick中断调用函数
//*******************************************************************************************/
//void TimingDelay_Decrement(void)
//{
//    if (TimingDelay != 0x00)
//    { 
//        TimingDelay--;
//    }
//}
void systick_init(void)
{
    RCC_ClocksTypeDef rcc_clocks;
    RCC_GetClocksFreq(&rcc_clocks);
    SysTick_Config(rcc_clocks.HCLK_Frequency/OS_TICKS_PER_SEC);
}


