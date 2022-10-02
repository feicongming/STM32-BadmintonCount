#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

void USART1_SendChar(uint8_t dat);               //发送单个字符
void USART1_SendMulti(uint8_t *dat,uint8_t len);   //发送多个字节
void USART1_SendString(uint8_t *dat);             //发送字符串
void USART_Config(void);     //串口1初始化函数
void Judge_ETB(void);       //判断是否接收完一帧数据
#endif
