#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"

void USART1_SendChar(uint8_t dat);               //���͵����ַ�
void USART1_SendMulti(uint8_t *dat,uint8_t len);   //���Ͷ���ֽ�
void USART1_SendString(uint8_t *dat);             //�����ַ���
void USART_Config(void);     //����1��ʼ������
void Judge_ETB(void);       //�ж��Ƿ������һ֡����
#endif
