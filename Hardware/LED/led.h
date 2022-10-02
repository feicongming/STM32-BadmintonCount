#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

#define LED_GPIO_CLK       RCC_APB2Periph_GPIOB
#define LED_GPIO_PORT      GPIOB
#define LED_GPIO_PIN       GPIO_Pin_10

#define LED_ON             GPIO_ResetBits(LED_GPIO_PORT, LED_GPIO_PIN)
#define LED_OFF            GPIO_SetBits(LED_GPIO_PORT, LED_GPIO_PIN)

void LED_Init(void);
void LED_PWM(u16 time_ms, u8 light);


#endif /* __LED_H */
