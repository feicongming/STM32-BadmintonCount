#ifndef __LM75DB_H
#define __LM75BD_H

#include "stm32f10x.h"
#include "delay.h"

//--------------------------------IIC端口定义----------------  					   			   
#define SCL_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_6)//SCL   置0
#define SCL_Set() GPIO_SetBits(GPIOB,GPIO_Pin_6)   //置1

#define SDA_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_7)//SDA  置0
#define SDA_Set() GPIO_SetBits(GPIOB,GPIO_Pin_7)  //置1
#define SDA_Read() GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)  //读取IO口外部电平状态


void SCL_GPIO_Init(void);   //将SCL引脚配置为推挽输出
void SDA_GPIO_Out_Init(void);  //将SDA引脚配置为推挽输出
void SDA_GPIO_Read_Init(void);  //将SDA引脚配置为 浮空输入
void LM75BD_IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_SendAck(unsigned char ackbit);   
unsigned char IIC_WaitAck(void); 
void IIC_SendByte(unsigned char byt);
unsigned char IIC_RecByte(void);
float LM75BD_ReadTemp(void);


#endif /* __LM75BD_H */

