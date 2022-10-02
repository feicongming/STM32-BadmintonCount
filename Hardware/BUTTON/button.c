#include "button.h"
#include "usart.h"


extern u8 button_event = 0;

void Button_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	// 使能GPIOA和AFIO时钟
	RCC_APB2PeriphClockCmd(KEY1_INT_GPIO_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	// 初始GPIO_Pin为上拉输入模式
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = KEY1_INT_GPIO_PIN | KEY2_INT_GPIO_PIN | ENCODER_A_GPIO_PIN | ENCODER_B_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	// 设置中断优先级
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_Init(&NVIC_InitStructure);

	// 初始外部中断
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource12);

    EXTI_InitStructure.EXTI_Line =  EXTI_Line0 | EXTI_Line11 | EXTI_Line12;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);

    delay_init();

}


void EXTI15_10_IRQHandler()
{
    // 判断标志位是否产生中断
    if(EXTI_GetITStatus(EXTI_Line11) != RESET)
    {
        USART1_SendString("BACK  ");
        button_event = KEY_BACK_EVENT;    // 返回键
        // delay_ms(5);
        EXTI_ClearITPendingBit(EXTI_Line11);
    }

    // 确定按键事件
    if(EXTI_GetITStatus(EXTI_Line12) != RESET)
    {
        USART1_SendString("ENTER  ");
        button_event = KEY_ENTER_EVENT;
        // delay_ms(1);
        EXTI_ClearITPendingBit(EXTI_Line12);
    }
}


// 旋转编码器下旋事件
void EXTI0_IRQHandler()
{
    delay_ms(3);
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0) button_event = ENCODER_DOWN_EVENT;
        else button_event = ENCODER_UP_EVENT;
        delay_ms(6);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}


// 旋转编码器上旋事件
// void EXTI1_IRQHandler()
// {
//     if(EXTI_GetITStatus(EXTI_Line1) != RESET)
//     {
//         delay_ms(5);
//         if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
//         {
//             button_event = ENCODER_UP_EVENT;
//         }
//         delay_ms(10);
//         EXTI_ClearITPendingBit(EXTI_Line1);
//     }
// }


/******************************************************************************
      函数说明：返回按键事件
      入口数据：无
      返回值：1为确认，2为返回，3为上转， 4为下转
******************************************************************************/
u8 Return_Button_Event(void)
{
    u8 temp = button_event;
    button_event = 0;
    return temp;
}
