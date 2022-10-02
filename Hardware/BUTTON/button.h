#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f10x.h"
#include "delay.h"


// 返回键和确认键
#define KEY1_INT_GPIO_CLK            RCC_APB2Periph_GPIOA
#define KEY1_INT_GPIO_PORT           GPIOA
#define KEY1_INT_GPIO_PIN            GPIO_Pin_11

#define KEY2_INT_GPIO_CLK            RCC_APB2Periph_GPIOA
#define KEY2_INT_GPIO_PORT           GPIOA
#define KEY2_INT_GPIO_PIN            GPIO_Pin_12

// 旋转编码器
#define ENCODER_INT_GPIO_CLK         RCC_APB2Periph_GPIOA
#define ENCODER_A_GPIO_PORT          GPIOA
#define ENCODER_A_GPIO_PIN           GPIO_Pin_0

#define ENCODER_INT_GPIO_CLK         RCC_APB2Periph_GPIOA
#define ENCODER_B_GPIO_PORT          GPIOA
#define ENCODER_B_GPIO_PIN           GPIO_Pin_1

// #define ENCODER_BUTTON_GPIO_CLK      RCC_APB2Periph_GPIOB
// #define ENCODER_BUTTON_GPIO_PORT     GPIOB
// #define ENCODER_BUTTON_GPIO_PIN      GPIO_Pin_11



// 定义按键事件
#define KEY_ENTER_EVENT        1
#define KEY_BACK_EVENT         2
#define ENCODER_UP_EVENT       3
#define ENCODER_DOWN_EVENT     4
// #define ENCODER_ENTER_EVENT    5

u8 Return_Button_Event(void);
void Button_Init(void);

#endif /* __BUTTON_H */
