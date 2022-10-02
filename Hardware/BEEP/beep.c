#include "beep.h"
#include "delay.h"


// 蜂鸣器初始化使能
void BEEP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 打开时钟
    RCC_APB2PeriphClockCmd(BEEP_GPIO_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = BEEP_GPIO_PIN;

    // 使能
    GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStructure);
    GPIO_SetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN);
}


/******************************************************************************
      函数说明：蜂鸣器蜂鸣
      入口数据： freq     发声频率
                time_ms  发声时间(ms)
                volume   音量(1~10之间)
      返回值：  无
******************************************************************************/
void BEEP(u16 freq, u16 time_ms, u8 volume)
{
    u16 count;
    u16 T_us;
    T_us = 1000000/freq;
    count = time_ms*freq/1000;
    while(count--)
    {
        GPIO_SetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN);
        delay_us((int)(T_us*((float)volume/20)));
        GPIO_ResetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN);
        delay_us((int)(T_us*(1-(float)volume/20)));
    }
    GPIO_SetBits(BEEP_GPIO_PORT, BEEP_GPIO_PIN);
}

