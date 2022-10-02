#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f10x.h"

#define BEEP_GPIO_CLK       RCC_APB2Periph_GPIOA
#define BEEP_GPIO_PORT      GPIOA
#define BEEP_GPIO_PIN       GPIO_Pin_8


void BEEP_Init(void);
void BEEP(u16 freq, u16 time_ms, u8 volume);


#endif /* __BEEP_H */
