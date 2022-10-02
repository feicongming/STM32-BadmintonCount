#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f10x.h"

#define KEY_INT_GPIO_CLK             RCC_APB2Periph_GPIOB
#define KEY_INT_GPIO_PORT            GPIOB
#define KEY1_INT_GPIO_PIN            GPIO_Pin_13
#define KEY2_INT_GPIO_PIN            GPIO_Pin_14

#define ENCODER_INT_GPIO_CLK         RCC_APB2Periph_GPIOA
#define ENCODER_INT_GPIO_PORT        GPIOA
#define ENCODER_A_GPIO_PIN           GPIO_Pin_11
#define ENCODER_B_GPIO_PIN           GPIO_Pin_12
#define ENCODER_BUTTON_GPIO_PIN      GPIO_Pin_10

#endif /* __BUTTON_H */