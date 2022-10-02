#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "sys.h"
#define USE_HORIZONTAL 3  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 240
#define LCD_H 320

#else
#define LCD_W 320
#define LCD_H 240
#endif



//-----------------LCD端口定义---------------- 

#define LCD_RES_Clr()  GPIO_ResetBits(GPIOA,GPIO_Pin_4)//RES
#define LCD_RES_Set()  GPIO_SetBits(GPIOA,GPIO_Pin_4)

#define LCD_DC_Clr()   GPIO_ResetBits(GPIOB,GPIO_Pin_1)//DC
#define LCD_DC_Set()   GPIO_SetBits(GPIOB,GPIO_Pin_1)
 		     
#define LCD_CS_Clr()   GPIO_ResetBits(GPIOA,GPIO_Pin_3)//CS
#define LCD_CS_Set()   GPIO_SetBits(GPIOA,GPIO_Pin_3)

#define LCD_BLK_Clr()  GPIO_ResetBits(GPIOA,GPIO_Pin_6)//BLK
#define LCD_BLK_Set()  GPIO_SetBits(GPIOA,GPIO_Pin_6)




void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(u8 dat);//模拟SPI时序
void LCD_WR_DATA8(u8 dat);//写入一个字节
void LCD_WR_DATA(u16 dat);//写入两个字节
void LCD_WR_REG(u8 dat);//写入一个指令
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
#endif




