#include "led.h"
#include "delay.h"


// LED初始化
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 打开时钟
    RCC_APB2PeriphClockCmd(LED_GPIO_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN;

    // 使能
    GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);

    // 关闭LED
    LED_OFF;
}


/******************************************************************************
      函数说明：LED可调亮度
      入口数据： time_ms  亮灯时间(ms)
                light    亮度(1~10之间)
      返回值：  无
******************************************************************************/
void LED_PWM(u16 time_ms, u8 light)
{
    u16 count = time_ms/20;
    while(count--)
    {
        LED_ON;
        delay_ms(2*light);
        LED_OFF;
        delay_ms(20-2*light);
    }
}
